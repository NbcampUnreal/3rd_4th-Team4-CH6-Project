// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/PlayerController/AO_PlayerController_MainMenu.h"
#include "UI/Widget/AO_MainMenuWidget.h"
#include "Kismet/GameplayStatics.h"
#include "AO/AO_Log.h"

void AAO_PlayerController_MainMenu::BeginPlay()
{
	Super::BeginPlay();

	AO_LOG(LogJSH, Log, TEXT("BeginPlay: MainMenu controller started"));

	if (MainMenu != nullptr)
	{
		AO_LOG(LogJSH, Warning, TEXT("BeginPlay: MainMenu already created, skipping"));
		return;
	}

	if (!MainMenuClass)
	{
		AO_LOG(LogJSH, Error, TEXT("BeginPlay: MainMenuClass not set on AO_PlayerController_MainMenu"));
		return;
	}

	MainMenu = CreateWidget<UAO_MainMenuWidget>(this, MainMenuClass);
	if (!MainMenu)
	{
		AO_LOG(LogJSH, Error, TEXT("BeginPlay: CreateWidget<UAO_MainMenuWidget> failed"));
		return;
	}

	MainMenu->AddToViewport(0);
	MainMenu->SetVisibility(ESlateVisibility::Visible);
	MainMenu->SetIsFocusable(true);

	AO_LOG(LogJSH, Log, TEXT("BeginPlay: MainMenu widget created and added to viewport"));

	FInputModeUIOnly Mode;
	Mode.SetWidgetToFocus(MainMenu->TakeWidget());
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(Mode);
	bShowMouseCursor = true;

	AO_LOG(LogJSH, Log, TEXT("BeginPlay: UIOnly input mode applied, mouse cursor enabled"));

	// JM 코드추가 : 세팅 위젯 생성
	if (SettingClass)
	{
		Settings = CreateWidget<UAO_UserWidget>(this, SettingClass);
		if (Settings)
		{
			Settings->AddToViewport(1);
			Settings->SetVisibility(ESlateVisibility::Hidden);
			AO_LOG(LogJM, Log, TEXT("Setting Widget Created"));
		}
		else
		{
			AO_LOG(LogJM, Warning, TEXT("Create Widget Failed"));
		}
	}
	else
	{
		AO_LOG(LogJM, Warning, TEXT("Setting Class is not set"));
	}
}
