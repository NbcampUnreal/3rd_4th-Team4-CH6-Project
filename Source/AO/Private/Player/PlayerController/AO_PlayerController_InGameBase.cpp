// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/PlayerController/AO_PlayerController_InGameBase.h"

#include "AO_DelegateManager.h"
#include "AO/AO_Log.h"
#include "UI/AO_UIStackManager.h"
#include "UI/Widget/AO_PauseMenuWidget.h"
#include "Online/AO_OnlineSessionSubsystem.h"
#include "Engine/GameInstance.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Character/AO_PlayerCharacter.h"
#include "Game/GameInstance/AO_GameInstance.h"
#include "Game/GameMode/AO_GameMode_InGameBase.h"
#include "GameFramework/GameStateBase.h"
#include "Interfaces/VoiceInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Player/Camera/AO_CameraManagerComponent.h"
#include "Player/PlayerState/AO_PlayerState.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "VoipListenerSynthComponent.h"
#include "UI/AO_UIActionKeySubsystem.h"

AAO_PlayerController_InGameBase::AAO_PlayerController_InGameBase()
{
	CameraManagerComponent = CreateDefaultSubobject<UAO_CameraManagerComponent>(TEXT("CameraManagerComponent"));
}

void AAO_PlayerController_InGameBase::AcknowledgePossession(APawn* P)
{
	Super::AcknowledgePossession(P);

	if (!IsLocalController())
	{
		return;
	}
	
	InitCameraManager(P);
}

void AAO_PlayerController_InGameBase::BeginPlay()
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	Super::BeginPlay();

	if (IsLocalPlayerController())
	{
		Client_StartVoiceChat_Implementation();	// 최초 입장 시 보이스 채팅 입력 활성화
	}

	if (IsLocalPlayerController())
	{
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UAO_UIActionKeySubsystem* Keys = GI->GetSubsystem<UAO_UIActionKeySubsystem>())
			{
				if (UInputMappingContext* IMC = Keys->GetUIIMC())
				{
					if (ULocalPlayer* LP = GetLocalPlayer())
					{
						if (UEnhancedInputLocalPlayerSubsystem* Subsys = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
						{
							Subsys->AddMappingContext(IMC, 100);
						}
					}
				}
			}
		}

		Client_StartVoiceChat_Implementation();
	}
		
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_PlayerController_InGameBase::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (InputComponent)
	{
		AO_LOG(LogJSH, Log, TEXT("InGameBase SetupInputComponent: Binding Test Keys on %s"), *GetName());

		// JM 코드추가 : 테스트용 키 직접 바인딩 (좋지 않음, 나중에 지워야함)
		InputComponent->BindKey(EKeys::Nine, IE_Pressed, this, &ThisClass::Test_Die);
		InputComponent->BindKey(EKeys::Zero, IE_Pressed, this, &ThisClass::Test_Alive);
	}
	
	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EIC)
	{
		return;
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UAO_UIActionKeySubsystem* Keys = GI->GetSubsystem<UAO_UIActionKeySubsystem>())
		{
			if (UInputAction* OpenAction = Keys->GetUIOpenAction())
			{
				EIC->BindAction(OpenAction, ETriggerEvent::Started, this, &ThisClass::HandleUIOpen);
			}
		}
	}
}

UAO_PauseMenuWidget* AAO_PlayerController_InGameBase::GetOrCreatePauseMenuWidget()
{
	if (IsValid(PauseMenu))
	{
		return PauseMenu;
	}

	if (!PauseMenuClass)
	{
		AO_LOG(LogJSH, Warning, TEXT("GetOrCreatePauseMenuWidget: PauseMenuClass not set on %s"), *GetName());
		return nullptr;
	}

	PauseMenu = CreateWidget<UAO_PauseMenuWidget>(this, PauseMenuClass);
	if (!PauseMenu)
	{
		AO_LOG(LogJSH, Error, TEXT("GetOrCreatePauseMenuWidget: Failed to create PauseMenu"));
		return nullptr;
	}

	// 델리게이트 바인딩 1회
	PauseMenu->OnRequestSettings.AddDynamic(this, &ThisClass::OnPauseMenu_RequestSettings);
	PauseMenu->OnRequestReturnLobby.AddDynamic(this, &ThisClass::OnPauseMenu_RequestReturnLobby);
	PauseMenu->OnRequestQuitGame.AddDynamic(this, &ThisClass::OnPauseMenu_RequestQuitGame);
	PauseMenu->OnRequestResume.AddDynamic(this, &ThisClass::OnPauseMenu_RequestResume);

	PauseMenu->SetIsFocusable(true);

	return PauseMenu;
}

void AAO_PlayerController_InGameBase::PreClientTravel(const FString& PendingURL, ETravelType TravelType,
	bool bIsSeamlessTravel)
{
	TObjectPtr<AAO_PlayerCharacter> PlayerCharacter = Cast<AAO_PlayerCharacter>(GetCharacter());
	checkf(PlayerCharacter, TEXT("Character is invalid"));

	PlayerCharacter->GetCustomizingComponent()->SaveCustomizingDataToPlayerState();
	
	Super::PreClientTravel(PendingURL, TravelType, bIsSeamlessTravel);
}

void AAO_PlayerController_InGameBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsCheckingVoiceCleanup)
	{
		AO_LOG_ROLE(LogJM, Log, TEXT("Inner bIsCheckingVoiceCleanup while Tick"));

		if (IsVoiceFullyCleanedUp())
		{
			bIsCheckingVoiceCleanup = false;
			Server_NotifyReadyForTravel();
			AO_LOG(LogJM, Log, TEXT("All Voice Resources Cleaned Up."));
		}
	}
}

void AAO_PlayerController_InGameBase::Client_StartVoiceChat_Implementation()
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	if (TObjectPtr<UAO_OnlineSessionSubsystem> OSSManager = GetGameInstance()->GetSubsystem<UAO_OnlineSessionSubsystem>())
	{
		OSSManager->StartVoiceChat();
	}
	else
	{
		AO_ENSURE(false, TEXT("OSS Manager is not Valid"));
	}
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_PlayerController_InGameBase::Client_StopVoiceChat_Implementation()
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	if (TObjectPtr<UAO_OnlineSessionSubsystem> OSSManager = GetGameInstance()->GetSubsystem<UAO_OnlineSessionSubsystem>())
	{
		OSSManager->StopVoiceChat();
	}
	else
	{
		AO_ENSURE(false, TEXT("OSS Manager is not Valid"));
	}
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_PlayerController_InGameBase::Client_UpdateVoiceMember_Implementation(AAO_PlayerState* DeadPlayerState)
{
	AO_LOG(LogJM, Log, TEXT("Start"));

	if (!AO_ENSURE(DeadPlayerState, TEXT("Dead PS is nullptr")))
	{
		return;
	}

	TObjectPtr<UAO_OnlineSessionSubsystem> OSS = GetGameInstance()->GetSubsystem<UAO_OnlineSessionSubsystem>();
	if (!AO_ENSURE(OSS, TEXT("OSS is nullptr")))
	{
		return;
	}

	TObjectPtr<AAO_PlayerState> AO_PS = Cast<AAO_PlayerState>(PlayerState);
	if (!AO_ENSURE(AO_PS, TEXT("Cast Failed PS -> AO_PS")))
	{
		return;
	}

	if (DeadPlayerState == AO_PS)	// 내가 죽은 경우, 다른 사람 모두 Unmute
	{
		TObjectPtr<UWorld> World = GetWorld();
		if (!AO_ENSURE(World, TEXT("No World")))
		{
			return;
		}

		for (TObjectPtr<APlayerState> OtherPS : World->GetGameState()->PlayerArray)
		{
			if (!AO_ENSURE(OtherPS->GetUniqueId().IsValid(), TEXT("PSId is Not Valid")))
			{
				continue;
			}

			TObjectPtr<AAO_PlayerState> AO_OtherPS = Cast<AAO_PlayerState>(OtherPS);
			if (!AO_ENSURE(AO_OtherPS, TEXT("Cast Failed PS -> AO_PS")))
			{
				continue;
			}

			if (OtherPS == AO_PS)	// JM : Ensure 생략, 본인은 unmute 작업을 생략하려는 의도
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

	TObjectPtr<AAO_PlayerState> AO_PS = Cast<AAO_PlayerState>(PlayerState);
	if (!AO_ENSURE(AO_PS, TEXT("Cast Failed PS -> AO_PS")))
	{
		return;
	}

	AO_PS->bIsAlive = false;	// 변경을 서버에서 해야함(권한), replicate 설정도 해줘야 함

	if (TObjectPtr<AAO_GameMode_InGameBase> AO_GameMode_InGame = Cast<AAO_GameMode_InGameBase>(GetWorld()->GetAuthGameMode()))
	{
		AO_GameMode_InGame->LetUpdateVoiceMemberForAllClients(this);
	}
	else
	{
		AO_ENSURE(false, TEXT("Cast Failed GM -> AO_GM_InGame"));
	}
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_PlayerController_InGameBase::Test_Server_SelfAlive_Implementation()
{
	AO_LOG(LogJM, Log, TEXT("Start"));

	TObjectPtr<AAO_PlayerState> AO_PS = Cast<AAO_PlayerState>(PlayerState);
	if (!AO_ENSURE(AO_PS, TEXT("Cast Failed PS -> AO_PS")))
	{
		return;
	}

	AO_PS->bIsAlive = true;	// 변경을 서버에서 해야함(권한), replicate 설정도 해줘야 함

	if (TObjectPtr<AAO_GameMode_InGameBase> AO_GameMode_InGame = Cast<AAO_GameMode_InGameBase>(GetWorld()->GetAuthGameMode()))
	{
		AO_GameMode_InGame->Test_LetUnmuteVoiceMemberForSurvivor(this);
	}
	else
	{
		AO_ENSURE(false, TEXT("Cast Failed GM -> AO_GM_InGame"));
	}
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_PlayerController_InGameBase::Client_UnmuteVoiceMember_Implementation(AAO_PlayerState* AlivePlayerState)
{
	AO_LOG(LogJM, Log, TEXT("Start"));

	if (!AO_ENSURE(AlivePlayerState, TEXT("Alive PS is InValid")))
	{
		return;
	}

	TObjectPtr<UAO_OnlineSessionSubsystem> OSS = GetGameInstance()->GetSubsystem<UAO_OnlineSessionSubsystem>();
	if (!AO_ENSURE(OSS, TEXT("OSS is Nullptr")))
	{
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

	TObjectPtr<AAO_PlayerState> AO_PS = Cast<AAO_PlayerState>(PlayerState);
	if (!AO_ENSURE(AO_PS, TEXT("Cast Failed PS -> AO_PS")))
	{
		return;
	}

	TObjectPtr<UWorld> World = GetWorld();
	if (!AO_ENSURE(World, TEXT("No World")))
	{
		return;
	}

	TObjectPtr<UAO_OnlineSessionSubsystem> OSS = GetGameInstance()->GetSubsystem<UAO_OnlineSessionSubsystem>();
	if (!AO_ENSURE(OSS, TEXT("OSS is Null")))
	{
		return;
	}

	// 사망자 모두 뮤트
	for (TObjectPtr<APlayerState> OtherPS : World->GetGameState()->PlayerArray)
	{
		if (!AO_ENSURE(OtherPS->GetUniqueId().IsValid(), TEXT("PSId is Not Valid")))
		{
			continue;
		}

		if (OtherPS == AO_PS)  // JM : Ensure 안씀. 본인은 mute 작업을 생략하려는 의도
		{
			AO_LOG(LogJM, Warning, TEXT("OtherPS == AO_PS"));
			continue;
		}

		TObjectPtr<AAO_PlayerState> AO_OtherPS = Cast<AAO_PlayerState>(OtherPS);
		if (!AO_ENSURE(AO_OtherPS, TEXT("Cast Failed PS -> AO_PS")))
		{
			continue;
		}

		if (AO_OtherPS->bIsAlive)	// 사망자만 mute 시킴
		{
			AO_LOG(LogJM, Warning, TEXT("AO_OtherPS is Survivor (Skip Mute)"));
			continue;
			// JM : Ensure 안씀. 생존자는 mute 작업을 생략하려는 의도
		}

		OSS->MuteRemoteTalker(0, AO_OtherPS, false);
	}

	Test_Server_SelfAlive();	// 생존자들이 나를 Unmute 하도록 ServerRPC 보냄

	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_PlayerController_InGameBase::Client_PrepareForTravel_Implementation(const FString& URL)
{
	AO_LOG(LogJM, Log, TEXT("Start (%s)"), *URL);
	
	CleanupAudioResource();
	bIsCheckingVoiceCleanup = true;
	
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_PlayerController_InGameBase::Server_NotifyReadyForTravel_Implementation()
{
	AO_LOG(LogJM, Log, TEXT("Start"));

	if (AAO_GameMode_InGameBase* GM_InGame = Cast<AAO_GameMode_InGameBase>(GetWorld()->GetAuthGameMode()))
	{
		GM_InGame->NotifyPlayerCleanupCompleteForTravel(this);
	}
	
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_PlayerController_InGameBase::CleanupAudioResource()
{
	AO_LOG(LogJM, Log, TEXT("Start"));

	if (UAO_OnlineSessionSubsystem* OSS = GetGameInstance()->GetSubsystem<UAO_OnlineSessionSubsystem>())
	{
		OSS->MuteAllRemoteTalker();	// JM : 안정성을 위해 Mute도 해주기
		OSS->StopVoiceChat();
	}
	else
	{
		AO_ENSURE(false, TEXT("Can't Get OSS"));
	}
	
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_PlayerController_InGameBase::HandleUIOpen()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UAO_UIStackManager* UIStack = GI->GetSubsystem<UAO_UIStackManager>())
		{
			// “열기”만 담당. 닫기는 UI_Close로 UI가 처리.
			UIStack->TryTogglePauseMenu(this);
		}
	}
}

void AAO_PlayerController_InGameBase::OnPauseMenu_RequestSettings()
{
	AO_LOG(LogJSH, Log, TEXT("Settings clicked"));
	AO_LOG(LogJM, Log, TEXT("Start"));

	if (TObjectPtr<UAO_DelegateManager> DelegateManager = GetGameInstance()->GetSubsystem<UAO_DelegateManager>())
	{
		DelegateManager->OnSettingsOpen.Broadcast();
		AO_LOG(LogJM, Log, TEXT("Broadcast OnSettingsOpen"));
	}
	else
	{
		AO_ENSURE(false, TEXT("Can't Get Delegate Manager"));
	}

	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_PlayerController_InGameBase::OnPauseMenu_RequestReturnLobby()
{
	if (UAO_OnlineSessionSubsystem* Sub = GetOnlineSessionSub())
	{
		// 세션을 떠나기 전에 GameInstance의 세션 데이터 초기화
		if (UWorld* World = GetWorld())
		{
			if (UAO_GameInstance* AO_GI = World->GetGameInstance<UAO_GameInstance>())
			{
				AO_GI->ResetSessionData();
			}
		}

		Sub->DestroyCurrentSession();
		return;
	}

	// 서브시스템 없으면 안전하게 메인 메뉴로
	if (UWorld* World = GetWorld())
	{
		if (UAO_GameInstance* AO_GI = World->GetGameInstance<UAO_GameInstance>())
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

void AAO_PlayerController_InGameBase::OnPauseMenu_RequestResume()
{
	// UIStackManager 단일 경로로 Pause 닫기
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UAO_UIStackManager* UIStack = GI->GetSubsystem<UAO_UIStackManager>())
		{
			UIStack->PopTop(this);
			return;
		}
	}

	AO_LOG(LogJSH, Warning, TEXT("UIStackManager not found. Resume ignored."));
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

void AAO_PlayerController_InGameBase::InitCameraManager(APawn* InPawn)
{
	checkf(CameraManagerComponent, TEXT("CameraManagerComponent not found"));
	
	AAO_PlayerCharacter* PlayerCharacter = Cast<AAO_PlayerCharacter>(InPawn);
	checkf(PlayerCharacter, TEXT("Character not found"));

	AO_LOG(LogKH, Log, TEXT("Called InitCameraManager"));

	CameraManagerComponent->BindCameraComponents(PlayerCharacter->GetSpringArm(), PlayerCharacter->GetCamera());
	CameraManagerComponent->PushCameraState(FGameplayTag::RequestGameplayTag(FName("Camera.Default")));

	if (UAbilitySystemComponent* ASC = PlayerCharacter->GetAbilitySystemComponent())
	{
		CameraManagerComponent->BindToASC(ASC);
	}
}

bool AAO_PlayerController_InGameBase::IsVoiceFullyCleanedUp()
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	UWorld* World = GetWorld();
	if (!AO_ENSURE(World, TEXT("No World")))
	{
		return true;
	}

	UAO_OnlineSessionSubsystem* OSS = GetGameInstance()->GetSubsystem<UAO_OnlineSessionSubsystem>();
	AGameStateBase* GS_Base = World->GetGameState<AGameStateBase>();
	if (OSS && GS_Base)
	{
		if (IOnlineVoicePtr VoiceInterface = OSS->GetOnlineVoiceInterface(); VoiceInterface.IsValid())
		{
			const FUniqueNetIdPtr LocalNetId = PlayerState->GetUniqueId().GetUniqueNetId();
			for (APlayerState* PS : GS_Base->PlayerArray)
			{
				if (!PS || PS == PlayerState)
				{
					continue;
				}

				if (FUniqueNetIdPtr RemoteNetId = PS->GetUniqueId().GetUniqueNetId(); RemoteNetId.IsValid())
				{
					if (!VoiceInterface->IsMuted(0, *RemoteNetId))		// NOTE : PlayerController::IsPlayerMuted(*RemoteNetId) 이거로는 체크 안됨 (무한 루프 돎)
					{
						AO_LOG(LogJM, Warning, TEXT("Wait: Player(%s) is not muted yet"), *RemoteNetId->ToString())
						return false;
					}
				}
			}
		}
	}

	for (TObjectIterator<UVoipListenerSynthComponent> It; It; ++It)
	{
		if (IsValid(*It) && It->IsRegistered())
		{
			AO_LOG(LogJM, Warning, TEXT("Wait: Engine is still unregistering %s"), *It->GetName());
			It->UnregisterComponent();
			return false;
		}
	}
	AO_LOG(LogJM, Log, TEXT("All remoteTalkers Muted & Component Unregistered"));
	return true;
}
