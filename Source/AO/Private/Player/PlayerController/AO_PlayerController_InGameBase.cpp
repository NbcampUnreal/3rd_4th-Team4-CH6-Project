// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/PlayerController/AO_PlayerController_InGameBase.h"

#include "AO/AO_Log.h"
#include "UI/Widget/AO_PauseMenuWidget.h"
#include "Online/AO_OnlineSessionSubsystem.h"
#include "Engine/GameInstance.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

AAO_PlayerController_InGameBase::AAO_PlayerController_InGameBase()
{
	bPauseMenuVisible = false;

	static ConstructorHelpers::FClassFinder<UAO_PauseMenuWidget> PauseMenuBPClass(
	TEXT("/Game/AVaOut/UI/Widget/WBP_PauseMenu")
	);
	if(PauseMenuBPClass.Succeeded())
	{
		PauseMenuClass = PauseMenuBPClass.Class;
	}
	else
	{
		AO_LOG(LogJSH, Warning, TEXT("PauseMenu widget not found, check path"));
	}
}

void AAO_PlayerController_InGameBase::BeginPlay()
{
	Super::BeginPlay();
}

void AAO_PlayerController_InGameBase::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	if (InputComponent)
	{
		AO_LOG(LogJSH, Log, TEXT("InGameBase SetupInputComponent: Binding ESC/P on %s"), *GetName());
		// ESC/P 키 테스트용 직접 바인딩
		InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &ThisClass::TogglePauseMenu);
		InputComponent->BindKey(EKeys::P, IE_Pressed, this, &ThisClass::TogglePauseMenu);
	}
}

void AAO_PlayerController_InGameBase::TogglePauseMenu()
{
	if (!PauseMenuClass)
	{
		AO_LOG(LogJSH, Warning, TEXT("PauseMenuClass not set on %s"), *GetName());
		return;
	}

	if (!PauseMenu)
	{
		PauseMenu = CreateWidget<UAO_PauseMenuWidget>(this, PauseMenuClass);
		if (!PauseMenu)
		{
			AO_LOG(LogJSH, Error, TEXT("Failed to create PauseMenu"));
			return;
		}

		PauseMenu->OnRequestSettings.AddDynamic(this, &ThisClass::OnPauseMenu_RequestSettings);
		PauseMenu->OnRequestReturnLobby.AddDynamic(this, &ThisClass::OnPauseMenu_RequestReturnLobby);
		PauseMenu->OnRequestQuitGame.AddDynamic(this, &ThisClass::OnPauseMenu_RequestQuitGame);
		PauseMenu->OnRequestResume.AddDynamic(this, &ThisClass::OnPauseMenu_RequestResume);
	}

	if (bPauseMenuVisible)
	{
		HidePauseMenu();
	}
	else
	{
		ShowPauseMenu();
	}
}

void AAO_PlayerController_InGameBase::ShowPauseMenu()
{
	if (!PauseMenu || bPauseMenuVisible)
	{
		return;
	}

	PauseMenu->AddToViewport(10);
	PauseMenu->SetVisibility(ESlateVisibility::Visible);
	PauseMenu->SetIsFocusable(true);

	FInputModeGameAndUI Mode;
	Mode.SetWidgetToFocus(PauseMenu->TakeWidget());
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	Mode.SetHideCursorDuringCapture(false);
	SetInputMode(Mode);

	bShowMouseCursor = true;
	bPauseMenuVisible = true;
}

void AAO_PlayerController_InGameBase::HidePauseMenu()
{
	if (!PauseMenu || !bPauseMenuVisible)
	{
		return;
	}

	PauseMenu->RemoveFromParent();

	FInputModeGameOnly Mode;
	SetInputMode(Mode);
	bShowMouseCursor = false;

	bPauseMenuVisible = false;
}

void AAO_PlayerController_InGameBase::OnPauseMenu_RequestSettings()
{
	// TODO: 설정 UI 완성되면 여기서 띄우기
	AO_LOG(LogJSH, Log, TEXT("Settings clicked (TODO)"));
}

void AAO_PlayerController_InGameBase::OnPauseMenu_RequestReturnLobby()
{
	HidePauseMenu();

	if (UAO_OnlineSessionSubsystem* Sub = GetOnlineSessionSub())
	{
		Sub->DestroyCurrentSession();
		return;
	}

	// 서브시스템 없으면 안전하게 메인 메뉴로
	if (UWorld* World = GetWorld())
	{
		UGameplayStatics::OpenLevel(World, FName(TEXT("/Game/AVaOut/Maps/LV_MainMenu")), true);
	}
}

void AAO_PlayerController_InGameBase::OnPauseMenu_RequestQuitGame()
{
	UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
}

UAO_OnlineSessionSubsystem* AAO_PlayerController_InGameBase::GetOnlineSessionSub() const
{
	if (const UGameInstance* GI = GetGameInstance())
	{
		if (UAO_OnlineSessionSubsystem* Sub = GI->GetSubsystem<UAO_OnlineSessionSubsystem>())
		{
			return Sub;
		}
	}

	AO_LOG(LogJSH, Warning, TEXT("GetOnlineSessionSub: OnlineSessionSubsystem not found"));
	return nullptr;
}

void AAO_PlayerController_InGameBase::OnPauseMenu_RequestResume()
{
	HidePauseMenu();
}
