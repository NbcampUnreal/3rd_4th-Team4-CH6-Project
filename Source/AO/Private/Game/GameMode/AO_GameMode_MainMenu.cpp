// Fill out your copyright...

#include "Game/GameMode/AO_GameMode_MainMenu.h"
#include "Player/PlayerController/AO_PlayerController_MainMenu.h"

AAO_GameMode_MainMenu::AAO_GameMode_MainMenu()
{
	PlayerControllerClass = AAO_PlayerController_MainMenu::StaticClass();
	DefaultPawnClass = nullptr;
	HUDClass = nullptr;
}
