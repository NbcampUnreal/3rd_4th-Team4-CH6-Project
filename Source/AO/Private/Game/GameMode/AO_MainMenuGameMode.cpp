// Fill out your copyright...

#include "Game/GameMode/AO_MainMenuGameMode.h"
#include "Player/PlayerController/AO_MainMenuPlayerController.h"

AAO_MainMenuGameMode::AAO_MainMenuGameMode()
{
	PlayerControllerClass = AAO_MainMenuPlayerController::StaticClass();
	DefaultPawnClass = nullptr;
	HUDClass = nullptr;
}
