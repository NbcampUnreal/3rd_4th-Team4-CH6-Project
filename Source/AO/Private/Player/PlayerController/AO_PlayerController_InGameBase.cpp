// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/PlayerController/AO_PlayerController_InGameBase.h"

#include "AO_DelegateManager.h"
#include "AO/AO_Log.h"
#include "UI/Widget/AO_PauseMenuWidget.h"
#include "Online/AO_OnlineSessionSubsystem.h"
#include "Engine/GameInstance.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Game/GameInstance/AO_GameInstance.h"
#include "Game/GameMode/AO_GameMode_InGameBase.h"
#include "GameFramework/GameStateBase.h"
#include "Interfaces/VoiceInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Player/PlayerState/AO_PlayerState.h"

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

	// TODO: Hard Load 말고 UPROPERTY를 활용한 로드로 전환 (WBP_Settings는 완료)
}

void AAO_PlayerController_InGameBase::BeginPlay()
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	Super::BeginPlay();

	if (IsLocalPlayerController())
	{
		AO_LOG(LogJM, Log, TEXT("Create"));
		CreateSettingsWidgetInstance(20, ESlateVisibility::Hidden);
		
		Client_StartVoiceChat_Implementation();	// 최초 입장 시 보이스 채팅 입력 활성화
	}
	AO_LOG(LogJM, Log, TEXT("End"));
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
		InputComponent->BindKey(EKeys::Nine, IE_Pressed, this, &ThisClass::Test_Die);
		InputComponent->BindKey(EKeys::Zero, IE_Pressed, this, &ThisClass::Test_Alive);
	}
}

void AAO_PlayerController_InGameBase::Client_StartVoiceChat_Implementation()
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	if (UAO_OnlineSessionSubsystem* OSSManager = GetGameInstance()->GetSubsystem<UAO_OnlineSessionSubsystem>())
	{
		OSSManager->StartVoiceChat();
	}
	else
	{
		AO_LOG(LogJM, Warning, TEXT("No OSS Manager"));
	}
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_PlayerController_InGameBase::Client_StopVoiceChat_Implementation()
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	if (UAO_OnlineSessionSubsystem* OSSManager = GetGameInstance()->GetSubsystem<UAO_OnlineSessionSubsystem>())
	{
		OSSManager->StopVoiceChat();
	}
	else
	{
		AO_LOG(LogJM, Warning, TEXT("No OSS Manager"));
	}
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_PlayerController_InGameBase::Client_UpdateVoiceMember_Implementation(AAO_PlayerState* DeadPlayerState)
{
	AO_LOG(LogJM, Log, TEXT("Start"));

	if (!DeadPlayerState)
	{
		AO_LOG(LogJM, Warning, TEXT("Dead PlayerState is null"));
		return;
	}

	UAO_OnlineSessionSubsystem* OSS = GetGameInstance()->GetSubsystem<UAO_OnlineSessionSubsystem>();
	if (!OSS)
	{
		AO_LOG(LogJM, Warning, TEXT("OSS is null"));
		return;
	}

	AAO_PlayerState* AO_PS = Cast<AAO_PlayerState>(PlayerState);
	if (!AO_PS)
	{
		AO_LOG(LogJM, Warning, TEXT("Cast Failed PS -> AO_PS"));
		return;
	}

	if (DeadPlayerState == AO_PS)	// 내가 죽은 경우, 다른 사람 모두 Unmute
	{
		UWorld* World = GetWorld();
		if (!World)
		{
			AO_LOG(LogJM, Warning, TEXT("No World"));
			return;
		}

		for (APlayerState* OtherPS : World->GetGameState()->PlayerArray)
		{
			if (!OtherPS->GetUniqueId().IsValid())
			{
				AO_LOG(LogJM, Warning, TEXT("PSId is Not Valid"));
				continue;
			}

			AAO_PlayerState* AO_OtherPS = Cast<AAO_PlayerState>(OtherPS);
			if (!AO_OtherPS)
			{
				AO_LOG(LogJM, Warning, TEXT("Cast Failed PS -> AO_PS"));
				continue;
			}
			
			if (OtherPS == AO_PS)
			{
				AO_LOG(LogJM, Warning, TEXT("OtherPS == AO_PS"));
				continue;
			}

			OSS->UnmuteRemoteTalker(0, AO_OtherPS, false);
		}
	}
	else if (AO_PS->bIsAlive)		// 내가 살아있다면, 죽은 사람 소리 Mute
	{
		OSS->MuteRemoteTalker(0, DeadPlayerState, false);
	}
	
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_PlayerController_InGameBase::Test_Server_SelfDie_Implementation()
{
	AO_LOG(LogJM, Log, TEXT("Start"));

	AAO_PlayerState* AO_PS = Cast<AAO_PlayerState>(PlayerState);
	if (!AO_PS)
	{
		AO_LOG(LogJM, Warning, TEXT("Cast Failed PS -> AO_PS"));
		return;
	}

	AO_PS->bIsAlive = false;	// 변경을 서버에서 해야함(권한), replicate 설정도 해줘야 함
	
	if (AAO_GameMode_InGameBase* AO_GameMode_InGame = Cast<AAO_GameMode_InGameBase>(GetWorld()->GetAuthGameMode()))
	{
		AO_GameMode_InGame->LetUpdateVoiceMemberForAllClients(this);
	}
	else
	{
		AO_LOG(LogJM, Warning, TEXT("Cast Failed GameMode -> AO_GameMode_InGame"))
	}
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_PlayerController_InGameBase::Test_Server_SelfAlive_Implementation()
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	if(GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, TEXT("Test_Server_SelfDie_Implementation Start"));

	AAO_PlayerState* AO_PS = Cast<AAO_PlayerState>(PlayerState);
	if (!AO_PS)
	{
		AO_LOG(LogJM, Warning, TEXT("Cast Failed PS -> AO_PS"));
		return;
	}

	AO_PS->bIsAlive = true;	// 변경을 서버에서 해야함(권한), replicate 설정도 해줘야 함
	
	if (AAO_GameMode_InGameBase* AO_GameMode_InGame = Cast<AAO_GameMode_InGameBase>(GetWorld()->GetAuthGameMode()))
	{
		AO_GameMode_InGame->Test_LetUnmuteVoiceMemberForSurvivor(this);
	}
	else
	{
		AO_LOG(LogJM, Warning, TEXT("Cast Failed GameMode -> AO_GameMode_InGame"))
	}
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_PlayerController_InGameBase::Client_UnmuteVoiceMember_Implementation(AAO_PlayerState* AlivePlayerState)
{
	AO_LOG(LogJM, Log, TEXT("Start"));

	if (!AlivePlayerState)
	{
		AO_LOG(LogJM, Warning, TEXT("InValid AlivePS"));
		return;
	}

	UAO_OnlineSessionSubsystem* OSS = GetGameInstance()->GetSubsystem<UAO_OnlineSessionSubsystem>();
	if (!OSS)
	{
		AO_LOG(LogJM, Warning, TEXT("OSS is Null"));
		return;
	}

	OSS->UnmuteRemoteTalker(0, AlivePlayerState, false);
	
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_PlayerController_InGameBase::Test_Die()
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	
	Test_Server_SelfDie();
	
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_PlayerController_InGameBase::Test_Alive()
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	
	AAO_PlayerState* AO_PS = Cast<AAO_PlayerState>(PlayerState);
	if (!AO_PS)
	{
		AO_LOG(LogJM, Warning, TEXT("Cast Failed PS -> AO_PS"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		AO_LOG(LogJM, Warning, TEXT("No World"));
		return;
	}

	UAO_OnlineSessionSubsystem* OSS = GetGameInstance()->GetSubsystem<UAO_OnlineSessionSubsystem>();
	if (!OSS)
	{
		AO_LOG(LogJM, Warning, TEXT("OSS is null"));
		return;
	}
	
	// 사망자 모두 뮤트
	for (APlayerState* OtherPS : World->GetGameState()->PlayerArray)
	{
		if (!OtherPS->GetUniqueId().IsValid())
		{
			AO_LOG(LogJM, Warning, TEXT("PSId is Not Valid"));
			continue;
		}
			
		if (OtherPS == AO_PS)
		{
			
			AO_LOG(LogJM, Warning, TEXT("OtherPS == AO_PS"));
			continue;
		}

		AAO_PlayerState* AO_OtherPS = Cast<AAO_PlayerState>(OtherPS);
		if (!AO_OtherPS)
		{
			AO_LOG(LogJM, Warning, TEXT("Cast Failed PS -> AO_PS"));
			continue;
		}

		if (AO_OtherPS->bIsAlive)	// 사망자만 mute 시킴
		{
			AO_LOG(LogJM, Warning, TEXT("AO_OtherPS is Survivor (Skip Mute)"));
			continue;
		}
			
		OSS->MuteRemoteTalker(0, AO_OtherPS, false);
	}
	
	Test_Server_SelfAlive();	// 생존자들이 나를 Unmute 하도록 ServerRPC 보냄
	
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
		// JM : ESC 누르면 설정창도 같이 닫히게 하기
		if (UAO_DelegateManager* DelegateManager = GetGameInstance()->GetSubsystem<UAO_DelegateManager>())
		{
			DelegateManager->OnSettingsClose.Broadcast();
		}
		else
		{
			AO_LOG(LogJM, Warning, TEXT("Can't Get Delegate Manager"));
		}
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

	// NOTE : 기존 상현님이 하셨던 대로 ZOrder 10으로 변경 (설정은 20으로)
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

	if(UAO_OnlineSessionSubsystem* Sub = GetOnlineSessionSub())
	{
		// 세션을 떠나기 전에 GameInstance의 세션 데이터 초기화
		if(UWorld* World = GetWorld())
		{
			if(UAO_GameInstance* AO_GI = World->GetGameInstance<UAO_GameInstance>())
			{
				AO_GI->ResetSessionData();
			}
		}

		Sub->DestroyCurrentSession();
		return;
	}

	// 서브시스템 없으면 안전하게 메인 메뉴로
	if(UWorld* World = GetWorld())
	{
		if(UAO_GameInstance* AO_GI = World->GetGameInstance<UAO_GameInstance>())
		{
			AO_GI->ResetSessionData();
		}

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
