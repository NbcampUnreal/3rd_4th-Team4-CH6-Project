// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/PlayerController/AO_PlayerController_Lobby.h"
#include "Game/GameMode/AO_GameMode_Lobby.h"
#include "AO/AO_Log.h"
#include "Player/PlayerState/AO_PlayerState.h"
#include "Engine/GameInstance.h"
#include "Online/AO_OnlineSessionSubsystem.h"

AAO_PlayerController_Lobby::AAO_PlayerController_Lobby()
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_PlayerController_Lobby::BeginPlay()
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	Super::BeginPlay();
	AO_LOG(LogJM, Log, TEXT("End"));

	// 메인 메뉴에서 넘어올 때 UIOnly 상태를 확실하게 정리
	FInputModeGameOnly Mode;
	SetInputMode(Mode);
	bShowMouseCursor = false;

	AO_LOG(LogJSH, Log, TEXT("Lobby PC BeginPlay: InputMode reset to GameOnly (%s)"), *GetName());
}

void AAO_PlayerController_Lobby::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	Super::EndPlay(EndPlayReason);
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_PlayerController_Lobby::Client_OpenInviteOverlay_Implementation()
{
	if(const UGameInstance* GI = GetGameInstance())
	{
		if(UAO_OnlineSessionSubsystem* Sub = GI->GetSubsystem<UAO_OnlineSessionSubsystem>())
		{
			Sub->ShowInviteUI();
			return;
		}
	}

	AO_LOG(LogJSH, Warning, TEXT("Client_OpenInviteOverlay: OnlineSessionSubsystem not found"));
}

void AAO_PlayerController_Lobby::Server_SetReady_Implementation(bool bNewReady)
{
	if(AAO_PlayerState* PS = GetPlayerState<AAO_PlayerState>())
	{
		PS->SetLobbyReady(bNewReady);
	}

	if (UWorld* World = GetWorld())
	{
		if (AAO_GameMode_Lobby* GM = World->GetAuthGameMode<AAO_GameMode_Lobby>())
		{
			GM->SetPlayerReady(this, bNewReady);
		}
	}
}

void AAO_PlayerController_Lobby::Server_RequestStart_Implementation()
{
	if (UWorld* World = GetWorld())
	{
		if (AAO_GameMode_Lobby* GM = World->GetAuthGameMode<AAO_GameMode_Lobby>())
		{
			GM->RequestStartFrom(this);
		}
	}
}