// AO_GameMode_Stage.cpp (장주만)


#include "Game/GameMode/AO_GameMode_Stage.h"
#include "Game/GameInstance/AO_GameInstance.h"
#include "AO_Log.h"

#include "Train/AO_Train.h"
#include "Online/AO_OnlineSessionSubsystem.h"
#include "AbilitySystemComponent.h"
#include "Train/GAS/AO_Fuel_AttributeSet.h"
#include "EngineUtils.h"
#include "Game/GameState/AO_GameState.h"
#include "GameFramework/CharacterMovementComponent.h"

AAO_GameMode_Stage::AAO_GameMode_Stage()
{
	AO_LOG(LogJM, Log, TEXT("Start"));
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
			}
		}
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
	
	if (bStageEnded)
	{
		return;
	}
	bStageEnded = true;

	UAO_GameInstance* AO_GI = World->GetGameInstance<UAO_GameInstance>();
	if(AO_GI == nullptr)
	{
		AO_LOG(LogJSH, Warning, TEXT("StageExit: GameInstance is not UAO_GameInstance"));
		return;
	}

	// 현재 스테이지의 열차에서 연료 값 가져오기
	AAO_Train* Train = nullptr;

	for(TActorIterator<AAO_Train> It(World); It; ++It)
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
	
	constexpr float RequiredFuel = 20.0f;

	if(Fuel < RequiredFuel)
	{
		AO_LOG(LogJSH, Log, TEXT("StageExit: Not enough fuel. Fuel=%.1f, Required=%.1f"), Fuel, RequiredFuel);
		return;
	}

	AO_LOG(LogJSH, Log, TEXT("StageExit: OK, Fuel=%.1f → Travel to next map"), Fuel);
	
	FName TargetMapName;

	// 마지막 스테이지면 → 로비로 귀환
	if(AO_GI->IsLastStage())
	{
		TargetMapName = AO_GI->GetLobbyMap();

		// 다음 판은 다시 처음부터
		AO_GI->ResetRun();
		RollbackSessionInGameFlag();
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
	World->ServerTravel(Path);
}

void AAO_GameMode_Stage::HandleStageFail(AController* Requester)
{
	if(!HasAuthority())
	{
		return;
	}
	
	if (bStageEnded)
	{
		return;
	}
	bStageEnded = true;

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

	AO_LOG(LogJSH, Log, TEXT("StageFail: Requested by %s, Fuel=%.1f, StageIndex=%d"),
		Requester ? *Requester->GetName() : TEXT("None"),
		AO_GI->SharedTrainFuel,
		AO_GI->CurrentStageIndex);

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

	World->ServerTravel(Path);
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
	if (AO_GI->TryConsumeSharedReviveCount() == false)
	{
		AO_LOG(LogJSH, Log, TEXT("TryRevivePlayer: no shared revive left"));
		return false;
	}

	// GameState에 최신 부활 카운트 동기화
	AO_GS->SetSharedReviveCount(AO_GI->GetSharedReviveCount());

	// 생존 플래그 되살리기
	AO_PS->SetIsAlive(true);

	// 기존 Pawn 정리 (넘어져 있는 시체/래그돌 제거)
	if (APawn* OldPawn = ReviveTargetPC->GetPawn())
	{
		OldPawn->DetachFromControllerPendingDestroy();
		OldPawn->Destroy();
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