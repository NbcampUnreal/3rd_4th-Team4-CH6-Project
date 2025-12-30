// JSH : AO_PlayerController_Rest.cpp

#include "Player/PlayerController/AO_PlayerController_Rest.h"

#include "Engine/World.h"
#include "Game/GameMode/AO_GameMode_Rest.h"

void AAO_PlayerController_Rest::Server_RequestRestExit_Implementation()
{
	UWorld* World = GetWorld();
	if(World == nullptr)
	{
		return;
	}

	AAO_GameMode_Rest* RestGM = World->GetAuthGameMode<AAO_GameMode_Rest>();
	if(RestGM == nullptr)
	{
		return;
	}

	RestGM->HandleRestExitRequest(this);
}
