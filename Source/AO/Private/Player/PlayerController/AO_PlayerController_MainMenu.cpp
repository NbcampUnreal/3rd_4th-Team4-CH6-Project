// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/PlayerController/AO_PlayerController_MainMenu.h"
#include "UI/Widget/AO_MainMenuWidget.h"
#include "Kismet/GameplayStatics.h"

void AAO_PlayerController_MainMenu::BeginPlay()
{
	Super::BeginPlay();

	if(MainMenu != nullptr)
	{
		return;
	}

	if(!MainMenuClass)
	{
		UE_LOG(LogTemp, Error, TEXT("MainMenuClass not set on AO_MainMenuPlayerController"));
		return;
	}

	MainMenu = CreateWidget<UAO_MainMenuWidget>(this, MainMenuClass);
	if(!MainMenu)
	{
		UE_LOG(LogTemp, Error, TEXT("CreateWidget<UAO_MainMenuWidget> failed"));
		return;
	}

	MainMenu->AddToViewport(0);
	MainMenu->SetVisibility(ESlateVisibility::Visible);
	MainMenu->SetIsFocusable(true);

	FInputModeUIOnly Mode;
	Mode.SetWidgetToFocus(MainMenu->TakeWidget());
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(Mode);
	bShowMouseCursor = true;
}