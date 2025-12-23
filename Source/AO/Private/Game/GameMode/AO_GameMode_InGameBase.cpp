// AO_GameMode_InGameBase.cpp (장주만)


#include "Game/GameMode/AO_GameMode_InGameBase.h"

#include "AO_Log.h"
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

void AAO_GameMode_InGameBase::LetUpdateVoiceMemberForAllClients(const TObjectPtr<AAO_PlayerController_InGameBase>& DeadPlayerController)
{
	AO_LOG(LogJM, Log, TEXT("Start"));

	TObjectPtr<AAO_PlayerState> DeadPlayerState = Cast<AAO_PlayerState>(DeadPlayerController->PlayerState);
	if (!AO_ENSURE(DeadPlayerState, TEXT("Cast Failed PS -> AAO_PS")))
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
				AO_PC_InGame->Client_UpdateVoiceMember(DeadPlayerState);
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
		StartServerTravel();	// 딜레이... 추가... ㅠㅠㅠ 아니 근데 안하면 죽어.. 오... 딜레이를 걸어도 죽는구먼! 하하하하 
		/*if (!GetWorld()->GetTimerManager().IsTimerActive(ServerTravelTimerHandle))
		{
			GetWorld()->GetTimerManager().SetTimer(ServerTravelTimerHandle, this, &ThisClass::StartServerTravel, 10.2f, false);
		}*/
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
	PendingTravelURL.Empty();	// PendingTravelURL = TEXT("");	/
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
