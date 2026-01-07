// AO_GameMode_Stage.cpp (장주만)


#include "Game/GameMode/AO_GameMode_Stage.h"
#include "Game/GameInstance/AO_GameInstance.h"
#include "AO_Log.h"
#include "Online/AO_OnlineSessionSubsystem.h"
#include "AbilitySystemComponent.h"
#include "Train/GAS/AO_Fuel_AttributeSet.h"
#include "EngineUtils.h"
#include "Game/AO_MapRoutes.h"
#include "Game/GameState/AO_GameState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/PlayerController/AO_PlayerController_Stage.h"
#include "Train/AO_newTrain.h"
#include "Train/Data/AO_FuelData.h"

AAO_GameMode_Stage::AAO_GameMode_Stage()
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	
	AutoReviveDelaySeconds = 5.0f;
	
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_GameMode_Stage::BeginPlay()
{
	AO_LOG(LogJM, Log, TEXT("BeginPlay Start"));
	Super::BeginPlay();

	if (HasAuthority())
	{
		UWorld* World = GetWorld();
		if (World != nullptr)
		{
			UAO_GameInstance* AO_GI = World->GetGameInstance<UAO_GameInstance>();
			AAO_GameState* AO_GS = GetGameState<AAO_GameState>();

			if (AO_GI != nullptr && AO_GS != nullptr)
			{
				const int32 ReviveCount = AO_GI->GetSharedReviveCount();
				AO_GS->SetSharedReviveCount(ReviveCount);
				AO_LOG(LogJSH, Log, TEXT("Stage BeginPlay: Sync SharedReviveCount GI(%d) -> GS"), ReviveCount);

				FString CurrentFullMapName = World->GetMapName();
				FString FirstStagePath = AO_MapRoutes::STAGE_MAPS[0];
				if (CurrentFullMapName.Contains(FPackageName::GetShortName(FirstStagePath)))	// 순수 맵 이름만 가져오기 FPackageName 활용
				{
					AO_GS->SetGameStartTime();
					AO_LOG(LogJM, Log, TEXT("Set Start Time"));
				}
			}
		}
		// 스테이지 시작 시 부활 대기 큐 초기화
		PendingReviveQueue.Empty();
	}

	AO_LOG(LogJM, Log, TEXT("BeginPlay End"));
}

void AAO_GameMode_Stage::PostLogin(APlayerController* NewPlayer)
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	Super::PostLogin(NewPlayer);
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_GameMode_Stage::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId,
	FString& ErrorMessage)
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_GameMode_Stage::HandleSeamlessTravelPlayer(AController*& C)
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	Super::HandleSeamlessTravelPlayer(C);
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_GameMode_Stage::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	Super::EndPlay(EndPlayReason);
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_GameMode_Stage::HandleStageExitRequest(AController* Requester)
{
	if(!HasAuthority())
	{
		return;
	}

	UWorld* World = GetWorld();
	if(World == nullptr)
	{
		return;
	}

	UAO_GameInstance* AO_GI = World->GetGameInstance<UAO_GameInstance>();
	if(AO_GI == nullptr)
	{
		AO_LOG(LogJSH, Warning, TEXT("StageExit: GameInstance is not UAO_GameInstance"));
		return;
	}

	AAO_GameState* GS = GetWorld()->GetGameState<AAO_GameState>();

	if (!GS->CheckHintCount())
	{
		AO_LOG(LogJSH, Warning, TEXT("StageExit: HintCount is not correct"));
		return;
	}
	bool AllHint = GS->CheckHintCount();
	if(AllHint==true)
	{
		AO_LOG(LogJSH, Log, TEXT("StageExit: All Hint is true"));
	}
	else
	{
		AO_LOG(LogJSH, Warning, TEXT("Not all hints are collected"));
		return;
	}
	
	// 현재 스테이지의 열차에서 연료 값 가져오기
	AAO_newTrain* Train = nullptr;

	for(TActorIterator<AAO_newTrain> It(World); It; ++It)
	{
		Train = *It;
		break; // 첫 번째 Train만 사용
	}

	if(Train == nullptr)
	{
		AO_LOG(LogJSH, Warning, TEXT("StageExit: Train not found in world"));
		return;
	}

	UAbilitySystemComponent* ASC = Train->GetAbilitySystemComponent();
	if(ASC == nullptr)
	{
		AO_LOG(LogJSH, Warning, TEXT("StageExit: Train has no AbilitySystemComponent"));
		return;
	}

	const float Fuel = ASC->GetNumericAttribute(UAO_Fuel_AttributeSet::GetFuelAttribute());
	
	// 가져온 연료를 GI에 1회 저장
	AO_GI->SharedTrainFuel = Fuel;
	
	const float RequiredFuel = GetRuquireFuelValue();

	if(Fuel < RequiredFuel)
	{
		AO_LOG(LogJSH, Log, TEXT("StageExit: Not enough fuel. Fuel=%.1f, Required=%.1f"), Fuel, RequiredFuel);
		return;
	}

	AO_GI->SharedTrainFuel = FMath::Max(0.0f, Fuel - RequiredFuel);
	AO_LOG(LogJSH, Log, TEXT("StageExit: OK, Fuel=%.1f → Travel to next map"), Fuel);
	
	if (bStageEnded)
	{
		return;
	}
	bStageEnded = true;
	
	FName TargetMapName;

	// 마지막 스테이지면 → 로비로 귀환
	if(AO_GI->IsLastStage())
	{
		AAO_GameState* AO_GS = GetGameState<AAO_GameState>();
		if (!AO_ENSURE(AO_GS, TEXT("Invalid AO_GS")))
		{
			return;
		}

		AO_GS->SetGameClear();	// JM : GS에서 OnRep 함수로 위젯을 띄우도록 함
		
		/* JM : 로직 겹쳐서 지움 (ui 에서 버튼 클릭하면 로비로 돌아가도록 할 예정)
		 *TargetMapName = AO_GI->GetLobbyMap();

		// 다음 판은 다시 처음부터
		AO_GI->ResetRun();
		RollbackSessionInGameFlag();*/
	}
	else
	{
		// 아니면 휴게공간으로 이동
		TargetMapName = AO_GI->GetRestMap();
	}

	if(TargetMapName.IsNone())
	{
		AO_LOG(LogJSH, Warning, TEXT("StageExit: TargetMapName is None"));
		return;
	}

	const FString Path = TargetMapName.ToString() + TEXT("?listen");
	// World->ServerTravel(Path);
	// JM : crash

	//ms 다음레벨 이동
	if (!GameState)
	{
		return;
	}

	if (AAO_GameState* AO_GS = Cast<AAO_GameState>(GameState))
	{
		for (APlayerState* P : AO_GS->PlayerArray)
		{
			if (AAO_PlayerState* AO_PS = Cast<AAO_PlayerState>(P))
			{
				AO_PS->bIsTraveling = true;
				
				if (APawn* MyPawn = AO_PS->GetPawn())
				{
					if (UAO_InventoryComponent* InvComp = MyPawn->FindComponentByClass<UAO_InventoryComponent>())
					{
						AO_PS->SaveInventoryBeforeTravel(InvComp);
					}
				}
				HandlePlayerTravel(AO_PS);
			}
		}
	}
	//ms
	
	RequestSynchronizedServerTravel(Path);
}

void AAO_GameMode_Stage::HandleStageFail(AController* Requester)
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	if (bStageEnded)
	{
		return;
	}
	bStageEnded = true;

	AAO_GameState* AO_GS = GetGameState<AAO_GameState>();
	if (!AO_ENSURE(AO_GS, TEXT("Invalid AO_GS")))
	{
		return;
	}

	AO_GS->SetStageFailed();	// JM : GS에서 OnRep 함수로 위젯을 띄우도록 함

	AO_LOG(LogJM, Log, TEXT("End"));
}

// TODO: JM : Requester 쓸 필요가없는데..? 그냥 지울까요?
void AAO_GameMode_Stage::ResetGameAndGoToLobby(AController* Requester)
{
	if(!HasAuthority())
	{
		return;
	}
	
	/*if (bStageEnded)
	{
		return;
	}
	bStageEnded = true;*/

	UWorld* World = GetWorld();
	if(World == nullptr)
	{
		return;
	}

	UAO_GameInstance* AO_GI = World->GetGameInstance<UAO_GameInstance>();
	if(AO_GI == nullptr)
	{
		AO_LOG(LogJSH, Warning, TEXT("StageFail: GameInstance is not UAO_GameInstance"));
		return;
	}

	/* 로그 찍으려고 Requester가 필요한건가?
	 *AO_LOG(LogJSH, Log, TEXT("StageFail: Requested by %s, Fuel=%.1f, StageIndex=%d"),
		Requester ? *Requester->GetName() : TEXT("None"),
		AO_GI->SharedTrainFuel,
		AO_GI->CurrentStageIndex);*/

	// 다음 판은 항상 처음부터
	AO_GI->ResetRun();
	RollbackSessionInGameFlag();

	const FName LobbyMapName = AO_GI->GetLobbyMap();
	if(LobbyMapName.IsNone())
	{
		AO_LOG(LogJSH, Warning, TEXT("StageFail: LobbyMapName is None"));
		return;
	}

	const FString Path = LobbyMapName.ToString() + TEXT("?listen");
	AO_LOG(LogJSH, Log, TEXT("StageFail: Travel to %s"), *Path);

	// World->ServerTravel(Path);
	// JM : crash 방지
	RequestSynchronizedServerTravel(Path);
}

void AAO_GameMode_Stage::TriggerStageFailByTrainFuel()
{
	if (HasAuthority() == false)
	{
		return;
	}

	AO_LOG(LogJSH, Log, TEXT("TriggerStageFailByTrainFuel: Train fuel below zero -> StageFail"));

	HandleStageFail(nullptr);
}

void AAO_GameMode_Stage::NotifyPlayerAliveStateChanged(AAO_PlayerState* ChangedPlayerState)
{
	if (HasAuthority() == false)
	{
		return;
	}

	if (ChangedPlayerState == nullptr)
	{
		return;
	}

	AO_LOG
	(
		LogJSH,
		Log,
		TEXT("NotifyPlayerAliveStateChanged: %s bIsAlive=%s"),
		*ChangedPlayerState->GetPlayerName(),
		ChangedPlayerState->GetIsAlive() ? TEXT("true") : TEXT("false")
	);
	
	// bIsAlive 변경에 따라 부활 대기 큐 관리
	if (ChangedPlayerState->GetIsAlive())
	{
		// 다시 살아난 경우 → 큐에서 제거
		// JSH: 자동 부활 큐에서의 제거는 TryAutoReviveFromQueue() 쪽에서만 처리
		//RemoveFromPendingRevive(ChangedPlayerState);
	}
	else
	{
		// 죽은 경우 → 큐에 추가
		EnqueuePendingRevive(ChangedPlayerState);

		// 팀 데스에 +1
		if (AAO_GameState* AO_GS = GetGameState<AAO_GameState>())
		{
			AO_GS->AddTeamDeathCount();
		}

		// 죽은 시점에 공용 부활 카운트가 남아 있다면 즉시 자동 부활 시도
		ScheduleAutoRevive();;
	}
	
	// JM : 캐릭터 생존 상태 변경 시, 모든 플레이어의 보이스 채팅 Mute 상태 업데이트 (논리적 분리)
	// JM : 이걸 여기서 하면 안돼고, PS에서 bIsAlive 값 OnRep 받아서 각자 업데이트 하도록 해야함
	/*if (AAO_PlayerController_InGameBase* AO_PC_InGameBase = Cast<AAO_PlayerController_InGameBase>(ChangedPlayerState->GetPlayerController()))
	{
		LetUpdateVoiceMemberForAllClients(AO_PC_InGameBase);
	}*/

	// 플레이어 한 명의 생존 상태가 바뀔 때마다 전멸 여부 재평가
	EvaluateTeamWipe();
}

/* 부활 관련 테스트 코드 */
bool AAO_GameMode_Stage::TryRevivePlayer(APlayerController* ReviveTargetPC)
{
	if (HasAuthority() == false || ReviveTargetPC == nullptr)
	{
		return false;
	}
	
	if (bStageEnded)
	{
		AO_LOG(LogJSH, Log, TEXT("TryRevivePlayer: Stage already ended, revive blocked."));
		return false;
	}

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return false;
	}

	UAO_GameInstance* AO_GI = World->GetGameInstance<UAO_GameInstance>();
	AAO_GameState* AO_GS = GetGameState<AAO_GameState>();
	if (AO_GI == nullptr || AO_GS == nullptr)
	{
		AO_LOG(LogJSH, Warning, TEXT("TryRevivePlayer: GI or GS is null"));
		return false;
	}

	AAO_PlayerState* AO_PS = ReviveTargetPC->GetPlayerState<AAO_PlayerState>();
	if (AO_PS == nullptr)
	{
		AO_LOG(LogJSH, Warning, TEXT("TryRevivePlayer: PlayerState is null"));
		return false;
	}

	// 이미 살아 있으면 부활 불필요
	if (AO_PS->GetIsAlive())
	{
		AO_LOG(LogJSH, Log, TEXT("TryRevivePlayer: already alive (%s)"), *ReviveTargetPC->GetName());
		return false;
	}

	// 부활 카운트 소모 (없으면 실패)
	if (AO_GS->TryConsumeSharedReviveCount() == false)
	{
		AO_LOG(LogJSH, Log, TEXT("TryRevivePlayer: no shared revive left"));
		return false;
	}

	// 생존 플래그 되살리기
	AO_PS->SetIsAlive(true);

	// 기존 Pawn 정리 (넘어져 있는 시체/래그돌 제거)
	if (APawn* OldPawn = ReviveTargetPC->GetPawn())
	{
		OldPawn->DetachFromControllerPendingDestroy();
		//OldPawn->Destroy();
	}

	// 시작 지점에서 리스폰 (기본 PlayerStart 사용)
	RestartPlayer(ReviveTargetPC);

	AO_LOG
	(
		LogJSH,
		Log,
		TEXT("TryRevivePlayer: revived %s, shared revive = %d"),
		*ReviveTargetPC->GetName(),
		AO_GI->GetSharedReviveCount()
	);

	return true;
}

bool AAO_GameMode_Stage::HasAnyAlivePlayer() const
{
	const AAO_GameState* AO_GS = GetGameState<AAO_GameState>();
	if (AO_GS == nullptr)
	{
		return false;
	}

	for (APlayerState* PS : AO_GS->PlayerArray)
	{
		AAO_PlayerState* AO_PS = Cast<AAO_PlayerState>(PS);
		if (AO_PS == nullptr)
		{
			continue;
		}

		if (AO_PS->GetIsAlive())
		{
			// 한 명이라도 살아 있으면 전멸 아님
			return true;
		}
	}

	// 모두 bIsAlive == false
	return false;
}

void AAO_GameMode_Stage::EvaluateTeamWipe()
{
	if (HasAuthority() == false)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	UAO_GameInstance* AO_GI = World->GetGameInstance<UAO_GameInstance>();
	if (AO_GI == nullptr)
	{
		AO_LOG(LogJSH, Warning, TEXT("EvaluateTeamWipe: GameInstance is not UAO_GameInstance"));
		return;
	}

	// 아직 살아 있는 플레이어가 하나라도 있으면 전멸 아님
	if (HasAnyAlivePlayer())
	{
		return;
	}

	// 공용 부활 횟수가 0이면 -> 진짜 전멸
	if (AO_GI->GetSharedReviveCount() <= 0)
	{
		AO_LOG(LogJSH, Log, TEXT("EvaluateTeamWipe: No alive players and no shared revive left -> StageFail"));

		// 누가 요청했는지 특정하기 어렵기 때문에 nullptr 전달
		HandleStageFail(nullptr);
	}
	else
	{
		// 모두 죽었지만 부활 횟수는 남아 있음 -> 일단 대기 (부활 Ability 등이 처리)
		AO_LOG(LogJSH, Log, TEXT("EvaluateTeamWipe: No alive players but revive left (%d) -> Wait"),
			AO_GI->GetSharedReviveCount());
	}
}

void AAO_GameMode_Stage::EnqueuePendingRevive(AAO_PlayerState* DeadPlayerState)
{
	if (HasAuthority() == false)
	{
		return;
	}

	if (DeadPlayerState == nullptr)
	{
		return;
	}

	for (const TWeakObjectPtr<AAO_PlayerState>& Entry : PendingReviveQueue)
	{
		if (Entry.Get() == DeadPlayerState)
		{
			return;
		}
	}

	PendingReviveQueue.Add(DeadPlayerState);

	AO_LOG
	(
		LogJSH,
		Log,
		TEXT("EnqueuePendingRevive: %s added. QueueSize=%d"),
		*DeadPlayerState->GetPlayerName(),
		PendingReviveQueue.Num()
	);
}

void AAO_GameMode_Stage::RemoveFromPendingRevive(AAO_PlayerState* PlayerState)
{
	if (HasAuthority() == false)
	{
		return;
	}

	if (PlayerState == nullptr)
	{
		return;
	}

	int32 Index = 0;

	while (Index < PendingReviveQueue.Num())
	{
		if (PendingReviveQueue[Index].Get() == PlayerState)
		{
			PendingReviveQueue.RemoveAt(Index);

			AO_LOG
			(
				LogJSH,
				Log,
				TEXT("RemoveFromPendingRevive: %s removed. QueueSize=%d"),
				*PlayerState->GetPlayerName(),
				PendingReviveQueue.Num()
			);

			continue;
		}

		++Index;
	}
}

void AAO_GameMode_Stage::TryAutoReviveFromQueue()
{
	if (HasAuthority() == false)
	{
		return;
	}

	if (bStageEnded)
	{
		return;
	}

	if (PendingReviveQueue.Num() <= 0)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	UAO_GameInstance* AO_GI = World->GetGameInstance<UAO_GameInstance>();
	AAO_GameState* AO_GS = GetGameState<AAO_GameState>();

	if (AO_GI == nullptr)
	{
		return;
	}

	if (AO_GS == nullptr)
	{
		return;
	}

	if (AO_GS->GetSharedReviveCount() <= 0)
	{
		return;
	}

	AO_LOG
	(
		LogJSH,
		Log,
		TEXT("TryAutoReviveFromQueue: Start. QueueSize=%d, SharedRevive=%d"),
		PendingReviveQueue.Num(),
		AO_GS->GetSharedReviveCount()
	);

	int32 Index = 0;

	while (Index < PendingReviveQueue.Num())
	{
		TWeakObjectPtr<AAO_PlayerState>& Entry = PendingReviveQueue[Index];
		AAO_PlayerState* DeadPS = Entry.Get();

		if (DeadPS == nullptr)
		{
			PendingReviveQueue.RemoveAt(Index);
			continue;
		}

		if (DeadPS->GetIsAlive())
		{
			PendingReviveQueue.RemoveAt(Index);
			continue;
		}

		if (AO_GS->GetSharedReviveCount() <= 0)
		{
			break;
		}

		APlayerController* DeadPC = DeadPS->GetPlayerController();
		if (DeadPC == nullptr)
		{
			++Index;
			continue;
		}

		const bool bSuccess = TryRevivePlayer(DeadPC);
		if (bSuccess)
		{
			if (AAO_PlayerController_Stage* StagePC = Cast<AAO_PlayerController_Stage>(DeadPC))
			{
				StagePC->Client_OnRevived();
			}

			PendingReviveQueue.RemoveAt(Index);
			continue;
		}
		else
		{
			break;
		}
	}

	AO_LOG
	(
		LogJSH,
		Log,
		TEXT("TryAutoReviveFromQueue: End. QueueSize=%d, SharedRevive=%d"),
		PendingReviveQueue.Num(),
		AO_GS->GetSharedReviveCount()
	);
}

void AAO_GameMode_Stage::ScheduleAutoRevive()
{
	if (HasAuthority() == false)
	{
		return;
	}

	if (AutoReviveDelaySeconds <= 0.0f)
	{
		// 딜레이 0 이하이면 기존처럼 즉시 자동 부활
		TryAutoReviveFromQueue();
		return;
	}

	// 매번 새로 설정해도 큰 문제는 없지만,
	// 혹시 이전 타이머가 있으면 한 번 정리
	GetWorldTimerManager().ClearTimer(AutoReviveTimerHandle);

	GetWorldTimerManager().SetTimer
	(
		AutoReviveTimerHandle,
		this,
		&AAO_GameMode_Stage::TryAutoReviveFromQueue,
		AutoReviveDelaySeconds,
		false
	);
}

void AAO_GameMode_Stage::HandleSharedReviveCountIncreased()
{
	if (HasAuthority() == false)
	{
		return;
	}

	if (bStageEnded)
	{
		return;
	}

	// 관전 중이며 큐에 쌓여 있던 플레이어들을 먼저 죽은 순서대로 자동 부활
	TryAutoReviveFromQueue();
}

void AAO_GameMode_Stage::RollbackSessionInGameFlag()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UGameInstance* GI = World->GetGameInstance();
	if (!GI)
	{
		return;
	}

	if (UAO_OnlineSessionSubsystem* Sub = GI->GetSubsystem<UAO_OnlineSessionSubsystem>())
	{
		Sub->SetSessionInGame(false);
	}
}

float AAO_GameMode_Stage::GetRuquireFuelValue()
{
	if (FuelDataAsset)
	{
		return FuelDataAsset->RequireNextStage;
	}
	return 40.0f;
}
