// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/PlayerController/AO_PlayerController_Lobby.h"
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