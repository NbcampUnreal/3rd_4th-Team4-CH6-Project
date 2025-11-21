// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/PlayerController/AO_PlayerController_InGameBase.h"

#include "AO_DelegateManager.h"
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

	// JM 코드추가 : 세팅위젯 로드
	// TODO: 왜 하드하게 로드하는 것이죠? 여기서 미리 넣어두면 Lobby나 InGame 계열에서는 따로 안해줘도 되는건가?
	static ConstructorHelpers::FClassFinder<UAO_UserWidget> SettingsWBPClass(TEXT("/Game/AVaOut/UI/Widget/Settings/WBP_Settings"));
	if(SettingsWBPClass.Succeeded())
	{
		SettingsClass = SettingsWBPClass.Class;
	}
	else
	{
		AO_LOG(LogJM, Warning, TEXT("Settings widget not found, check path"));
	}
}

void AAO_PlayerController_InGameBase::BeginPlay()
{
	Super::BeginPlay();

	if (SettingsClass)
	{
		Settings = CreateWidget<UAO_UserWidget>(this, SettingsClass);
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

void AAO_PlayerController_InGameBase::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	if (InputComponent)
	{
		AO_LOG(LogJSH, Log, TEXT("InGameBase SetupInputComponent: Binding ESC/P on %s"), *GetName());
		// ESC/P 키 테스트용 직접 바인딩
		InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &ThisClass::TogglePauseMenu);
		InputComponent->BindKey(EKeys::P, IE_Pressed, this, &ThisClass::TogglePauseMenu);

		// JM 코드추가 : 테스트용 키 직접 바인딩 (좋지 않음, 나중에 지워야함)
		InputComponent->BindKey(EKeys::Nine, IE_Pressed, this, &ThisClass::JMTestOpenSettings);
		InputComponent->BindKey(EKeys::Zero, IE_Pressed, this, &ThisClass::JMTestCloseSettings);
	}
}

void AAO_PlayerController_InGameBase::JMTestOpenSettings()	// JM 코드추가 : 테스트용 코드
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	if(UAO_DelegateManager* DelegateManager = GetGameInstance()->GetSubsystem<UAO_DelegateManager>())
	{
		DelegateManager->OnSettingsOpen.Broadcast();
	}
	else
	{
		AO_LOG(LogJM, Warning, TEXT("No DelegateManager"));
	}
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_PlayerController_InGameBase::JMTestCloseSettings()	// JM 코드추가 : 테스트용 코드
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	if(UAO_DelegateManager* DelegateManager = GetGameInstance()->GetSubsystem<UAO_DelegateManager>())
	{
		DelegateManager->OnSettingsClose.Broadcast();
	}
	else
	{
		AO_LOG(LogJM, Warning, TEXT("No DelegateManager"));
	}
	AO_LOG(LogJM, Log, TEXT("End"));
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

	// PauseMenu->AddToViewport(10);		// TODO: JM : 어... 상현님 이거 왜 10이죠?
	// TODO : JM : 지금 다 일반호출이라서, 나중에 델리게이트 기반으로 호출 바꿔야 할 수도 있을듯요
	PauseMenu->AddToViewport(0);		// JM : 메뉴가 설정화면보다 위로 올라와서 zorder 내림
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

	// JM 코드추가 : 아.... 위젯에서 델리게이트를 호출해서 이 함수를 실행시키는구나..
	AO_LOG(LogJM, Log, TEXT("Start"));
	if (UAO_DelegateManager* DelegateManager = GetGameInstance()->GetSubsystem<UAO_DelegateManager>())
	{
		DelegateManager->OnSettingsOpen.Broadcast();
		AO_LOG(LogJM, Log, TEXT("Broadcast OnSettingsOpen"));
	}
	else
	{
		AO_LOG(LogJM, Warning, TEXT("Cant Get DelegateManager"));
	}
	AO_LOG(LogJM, Log, TEXT("End"));
	
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
