// AO_PlayerController_Stage.cpp (장주만)


#include "Player/PlayerController/AO_PlayerController_Stage.h"
#include "Game/GameMode/AO_GameMode_Stage.h"
#include "Game/GameInstance/AO_GameInstance.h"
#include "AO_Log.h"



void AAO_PlayerController_Stage::Server_RequestStageExit_Implementation()
{
	AO_LOG(LogJSH, Log, TEXT("Server_RequestStageExit"));

	if(UWorld* World = GetWorld())
	{
		if(AAO_GameMode_Stage* GM = World->GetAuthGameMode<AAO_GameMode_Stage>())
		{
			GM->HandleStageExitRequest(this);
		}
		else
		{
			AO_LOG(LogJSH, Warning, TEXT("Server_RequestStageExit: GameMode_Stage not found"));
		}
	}
}
