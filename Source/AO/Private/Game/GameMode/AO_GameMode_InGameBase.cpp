// AO_GameMode_InGameBase.cpp (장주만)


#include "Game/GameMode/AO_GameMode_InGameBase.h"

#include "AO_Log.h"
#include "EngineUtils.h"
#include "Character/AO_PlayerCharacter.h"
#include "Components/BoxComponent.h"
#include "Item/InventoryTravelSafeZone/AO_InventorySaveZone.h"
#include "Item/PassiveContainer/AO_Passive_WorldSubsystem.h"
#include "Player/PlayerController/AO_PlayerController_InGameBase.h"
#include "Player/PlayerState/AO_PlayerState.h"

AAO_GameMode_InGameBase::AAO_GameMode_InGameBase()
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	bUseSeamlessTravel = true;
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_GameMode_InGameBase::HandleSeamlessTravelPlayer(AController*& C)
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	
	Super::HandleSeamlessTravelPlayer(C);

	// LetStartVoiceChat(C);	// 강제로 실행할 필요 없음. 그냥 사용자 설정에 맞게 반영	// 레벨 이동시 VoiceChat이 종료되어서 다시 실행시킴
	
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_GameMode_InGameBase::HandlePlayerTravel(AAO_PlayerState* PS)
{
	if (!PS) return;

	// 1. 데이터 확보: 캐릭터 인벤토리를 PS로 복사
	if (APawn* Pawn = PS->GetPawn())
	{
		if (UAO_InventoryComponent* Inv = Pawn->FindComponentByClass<UAO_InventoryComponent>())
		{
			PS->SaveInventoryBeforeTravel(Inv);
		}
	}

	// 1-2. 데이터 확보: 캐릭터 Health를 PS로 복사
	if (APawn* Pawn = PS->GetPawn())
	{
		if (AAO_PlayerCharacter* PlayerCharacter = Cast<AAO_PlayerCharacter>(Pawn))
		{
			const float CurrentHealth = PlayerCharacter->GetCurrentHealth();

			PS->SaveHealthBeforeTravel(CurrentHealth);
		}
	}

	// 2. 판정: 휴게소는 무조건 통과, 스테이지는 좌표 체크
	bool bFinalPersist = false;
	FString MapName = GetWorld()->GetMapName();

	if (MapName.Contains(TEXT("Rest")))
	{
		bFinalPersist = true;
		UE_LOG(LogTemp, Log, TEXT("[Travel] 휴게소 판정: 무조건 유지"));
	}
	else
	{
		bFinalPersist = CheckPlayerInsideSafeZone(PS);
	}

	// 3. 결과 반영
	PS->bInventoryShouldPersist = bFinalPersist;

	if (PS->bInventoryShouldPersist)
	{
		UE_LOG(LogTemp, Log, TEXT("[Travel] 최종 판정 유지: InvCount(%d)"), PS->PersistentInventory.Num());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Travel] 최종 판정 삭제: 범위 밖"));
		PS->PersistentInventory.Empty();
	}
}

void AAO_GameMode_InGameBase::StopVoiceChatForAllClients() const
{
	AO_LOG(LogJM, Log, TEXT("Start"));

	TObjectPtr<UWorld> World = GetWorld();
	if (!AO_ENSURE(World, TEXT("World is Not valid")))
	{
		return;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		if (TObjectPtr<AAO_PlayerController_InGameBase> AO_PC_InGame = Cast<AAO_PlayerController_InGameBase>(*It))
		{
			AO_PC_InGame->Client_StopVoiceChat();
		}
		else
		{
			AO_ENSURE(false, TEXT("Can't Cast to PC -> PC_InGame"));
		}
	}
	
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_GameMode_InGameBase::LetUpdateVoiceMemberForAllClients(const TObjectPtr<AAO_PlayerController_InGameBase>& ChangedPlayerController)
{
	AO_LOG(LogJM, Log, TEXT("Start"));

	TObjectPtr<AAO_PlayerState> ChangedPlayerState = Cast<AAO_PlayerState>(ChangedPlayerController->PlayerState);
	if (!AO_ENSURE(ChangedPlayerState, TEXT("Cast Failed PS -> AAO_PS")))
	{
		return;
	}

	TObjectPtr<UWorld> World = GetWorld();
	if (!AO_ENSURE(World, TEXT("World is not Valid")))
	{
		return;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		if (TObjectPtr<APlayerController> PC = It->Get())
		{
			if (TObjectPtr<AAO_PlayerController_InGameBase> AO_PC_InGame = Cast<AAO_PlayerController_InGameBase>(PC))
			{
				AO_PC_InGame->Client_UpdateVoiceMember(ChangedPlayerState);
			}
			else
			{
				AO_ENSURE(false, TEXT("Can't Cast to PC -> PC_InGame"));
			}
		}
		else
		{
			AO_ENSURE(false, TEXT("PC from Iterator is not Valid"));
		}
	}
	
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_GameMode_InGameBase::Test_LetUnmuteVoiceMemberForSurvivor(const TObjectPtr<AAO_PlayerController_InGameBase>& AlivePC)
{
	AO_LOG(LogJM, Log, TEXT("Start"));

	TObjectPtr<AAO_PlayerState> AlivePlayerState = Cast<AAO_PlayerState>(AlivePC->PlayerState); 
	if (!AO_ENSURE(AlivePlayerState, TEXT("Cast Failed PS -> AAO_PS")))
	{
		return;
	}

	TObjectPtr<UWorld> World = GetWorld();
	if (!AO_ENSURE(World, TEXT("World is not Valid")))
	{
		return;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		if (TObjectPtr<APlayerController> PC = It->Get())
		{
			if (TObjectPtr<AAO_PlayerController_InGameBase> AO_PC_InGame = Cast<AAO_PlayerController_InGameBase>(PC))
			{
				if (TObjectPtr<AAO_PlayerState> AO_PS = AO_PC_InGame->GetPlayerState<AAO_PlayerState>())
				{
					if (AO_PS->bIsAlive)
					{
						AO_PC_InGame->Client_UnmuteVoiceMember(AlivePlayerState);
					}
				}
				else
				{
					AO_ENSURE(false, TEXT("Failed to get PS from valid AO_PC_InGame"));
				}
			}
			else
			{
				AO_ENSURE(false, TEXT("Can't Cast to PC -> PC_InGame"));
			}
		}
		else
		{
			AO_ENSURE(false, TEXT("PC from Iterator is not Valid"));
		}
	}
	
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_GameMode_InGameBase::RequestSynchronizedServerTravel(const FString& URL)
{
	AO_LOG(LogJM, Log, TEXT("Start(URL : %s"), *URL);
	if (bIsTravelSyncInProgress)
	{
		return;
	}

	bIsTravelSyncInProgress = true;
	PendingTravelURL = URL;
	CleanupCompletePlayers.Empty();

	UWorld* World = GetWorld();
	if (!AO_ENSURE(World, TEXT("World is not Valid")))
	{
		return;
	}
	
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		if (AAO_PlayerController_InGameBase* PC = Cast<AAO_PlayerController_InGameBase>(It->Get()))
		{
			//ms 인벤토리 저장
			// 모든 플레이어에 대해 트래블 준비 실행
			if (AAO_PlayerState* PS = PC->GetPlayerState<AAO_PlayerState>())
			{
				PS->bIsTraveling = true;
				HandlePlayerTravel(PS); // 여기서 저장+판정 한 번에 처리
			}
			//ms
			PC->Client_PrepareForTravel(URL);
		}
	}
	
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_GameMode_InGameBase::NotifyPlayerCleanupCompleteForTravel(AAO_PlayerController* PC)
{
	AO_LOG(LogJM, Log, TEXT("Start"));

	if (!bIsTravelSyncInProgress || !PC)
	{
		return;
	}
	
	CleanupCompletePlayers.Add(PC);
	int32 TotalPlayers = GetNumPlayers();
	AO_LOG(LogJM, Log, TEXT("Player CleanupComplete : %s (%d / %d)"), *PC->GetName(), CleanupCompletePlayers.Num(), TotalPlayers);

	if (CleanupCompletePlayers.Num() >= TotalPlayers)
	{
		StartServerTravel();	// NOTE : Delay를 주는 방법으로는 해결할 수 없음
	}
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_GameMode_InGameBase::StartServerTravel()
{
	AO_LOG(LogJM, Log, TEXT("Start"));

	UWorld* World = GetWorld();
	if (World && !PendingTravelURL.IsEmpty())
	{
		World->ServerTravel(PendingTravelURL);
	}

	bIsTravelSyncInProgress = false;
	PendingTravelURL.Empty();
	CleanupCompletePlayers.Empty();
	
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_GameMode_InGameBase::LetStartVoiceChat(AController*& TargetController)
{
	AO_LOG(LogJM, Log, TEXT("Start"));

	if (TObjectPtr<AAO_PlayerController_InGameBase> AO_PC_InGame = Cast<AAO_PlayerController_InGameBase>(TargetController))
	{
		AO_PC_InGame->Client_StartVoiceChat();
	}
	else
	{
		AO_ENSURE(false, TEXT("Can't Cast Controller -> AO_PC_InGame"));
	}
	
	AO_LOG(LogJM, Log, TEXT("End"));
}

//ms : 인벤토리 유지 영역 확인
bool AAO_GameMode_InGameBase::CheckPlayerInsideSafeZone(class AAO_PlayerState* PS)
{
	if (!PS || !PS->GetPawn()) return false;

	FVector PlayerLocation = PS->GetPawn()->GetActorLocation();
	
	for (TActorIterator<AAO_InventorySaveZone> It(GetWorld()); It; ++It)
	{
		if (AAO_InventorySaveZone* SafeZone = *It)
		{
			if (UBoxComponent* BoxComp = SafeZone->FindComponentByClass<UBoxComponent>())
			{
				if (BoxComp->Bounds.GetBox().IsInside(PlayerLocation))
				{
					return true;
				}
			}
		}
	}
	return false;
}
