// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/GameMode/AO_GameMode_Lobby.h"
#include "Player/PlayerController/AO_PlayerController_Lobby.h"

AAO_GameMode_Lobby::AAO_GameMode_Lobby()
{
	PlayerControllerClass = AAO_PlayerController_Lobby::StaticClass();
	// bUseSeamlessTravel = false; // 나중에 필요하면 켜기
}
