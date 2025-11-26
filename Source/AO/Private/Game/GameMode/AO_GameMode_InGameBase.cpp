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

	LetStartVoiceChat(C);		// 레벨 이동시 VoiceChat이 종료되어서 다시 실행시킴
	
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_GameMode_InGameBase::StopVoiceChatForAllClients() const
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	if (UWorld* World = GetWorld())
	{
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			if (AAO_PlayerController_InGameBase* AO_PC_InGame = Cast<AAO_PlayerController_InGameBase>(*It))
			{
				AO_PC_InGame->Client_StopVoiceChat();
			}
			else
			{
				AO_LOG(LogJM, Warning, TEXT("Can't Cast to PC -> PC_InGame"));
			}
		}
	}
	else
	{
		AO_LOG(LogJM, Warning, TEXT("No World"));
	}
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_GameMode_InGameBase::LetUpdateVoiceMemberForAllClients(const AAO_PlayerController_InGameBase* DeadPlayerController)
{
	AO_LOG(LogJM, Log, TEXT("Start"));

	AAO_PlayerState* DeadPlayerState = Cast<AAO_PlayerState>(DeadPlayerController->PlayerState);
	if (!DeadPlayerState)
	{
		AO_LOG(LogJM, Warning, TEXT("Cast Failed PS -> DeadPlayerState"))
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		AO_LOG(LogJM, Warning, TEXT("World is not Valid"));
		return;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* PC = It->Get())
		{
			if (AAO_PlayerController_InGameBase* AO_PC_InGame = Cast<AAO_PlayerController_InGameBase>(PC))
			{
				AO_PC_InGame->Client_UpdateVoiceMember(DeadPlayerState);
			}
			else
			{
				AO_LOG(LogJM, Warning, TEXT("Can't Cast to PC -> PC_InGame"));
			}
		}
		else
		{
			AO_LOG(LogJM, Warning, TEXT("PC is not Valid"))
		}
	}
	
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_GameMode_InGameBase::LetStartVoiceChat(AController*& TargetController)
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	
	if (AAO_PlayerController_InGameBase* AO_PC_InGame = Cast<AAO_PlayerController_InGameBase>(TargetController))
	{
		AO_PC_InGame->Client_StartVoiceChat();
	}
	else
	{
		AO_LOG(LogJM, Warning, TEXT("Can't Cast to Controller -> PC_InGame"));
	}
	
	AO_LOG(LogJM, Log, TEXT("End"));
}
