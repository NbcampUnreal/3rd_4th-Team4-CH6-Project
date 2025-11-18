// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/PlayerController/AO_PlayerController_Lobby.h"
#include "Game/GameMode/AO_GameMode_Lobby.h"
#include "AO/AO_Log.h"

AAO_PlayerController_Lobby::AAO_PlayerController_Lobby()
{
}

void AAO_PlayerController_Lobby::BeginPlay()
{
	Super::BeginPlay();

	// 메인 메뉴에서 넘어올 때 UIOnly 상태를 확실하게 정리
	FInputModeGameOnly Mode;
	SetInputMode(Mode);
	bShowMouseCursor = false;

	AO_LOG(LogJSH, Log, TEXT("Lobby PC BeginPlay: InputMode reset to GameOnly (%s)"), *GetName());
}

void AAO_PlayerController_Lobby::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (InputComponent)
	{
		// 2키: 레디 토글
		InputComponent->BindKey(EKeys::Two, IE_Pressed, this, &ThisClass::OnPressed_ReadyKey);

		// 3키: 호스트 시작 요청
		InputComponent->BindKey(EKeys::Three, IE_Pressed, this, &ThisClass::OnPressed_StartKey);
	}
}

void AAO_PlayerController_Lobby::OnPressed_ReadyKey()
{
	bIsReady = !bIsReady;

	AO_LOG(LogJSH, Log, TEXT("LobbyPC: Ready key pressed (%s), NewReady=%d"),
		*GetName(),
		static_cast<int32>(bIsReady));

	ServerSetReady(bIsReady);
}

void AAO_PlayerController_Lobby::OnPressed_StartKey()
{
	AO_LOG(LogJSH, Log, TEXT("LobbyPC: Start key pressed (%s)"), *GetName());

	ServerRequestStart();
}

void AAO_PlayerController_Lobby::ServerSetReady_Implementation(bool bNewReady)
{
	bIsReady = bNewReady;

	if (UWorld* World = GetWorld())
	{
		if (AAO_GameMode_Lobby* GM = World->GetAuthGameMode<AAO_GameMode_Lobby>())
		{
			GM->SetPlayerReady(this, bNewReady);
		}
	}
}

void AAO_PlayerController_Lobby::ServerRequestStart_Implementation()
{
	if (UWorld* World = GetWorld())
	{
		if (AAO_GameMode_Lobby* GM = World->GetAuthGameMode<AAO_GameMode_Lobby>())
		{
			GM->RequestStartFrom(this);
		}
	}
}