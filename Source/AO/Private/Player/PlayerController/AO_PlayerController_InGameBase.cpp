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

void AAO_PlayerController_InGameBase::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (!IsLocalController())
	{
		return;
	}

	//InitCameraManager();
}

void AAO_PlayerController_InGameBase::OnRep_Pawn()
{
	Super::OnRep_Pawn();

	if (!IsLocalController())
	{
		return;
	}

	//InitCameraManager();
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
		
		/*// 1. OSS 내부 상태 확인 (사용자 제안 방식)
		IOnlineVoicePtr VoiceInt = GetGameInstance()->GetSubsystem<UAO_OnlineSessionSubsystem>()->GetOnlineVoiceInterface();
		if (VoiceInt.IsValid())
		{
			TArray<FUniqueNetIdWrapper> ToRemove;
			for (const FUniqueNetIdWrapper& TargetID : PendingUnregisterIDs)
			{
				// Unregister를 시도했을 때 false가 나오면 이미 목록에 없는 것임
				if (TargetID.IsValid() && !VoiceInt->UnregisterRemoteTalker(*TargetID))
				{
					AO_LOG(LogJM, Warning, TEXT("UnregisterRemoteTalker Failed: %s (즉, 이미 unregister 한 녀석임)"), *TargetID->ToString());
					ToRemove.Add(TargetID);
				}
			}

			for (const FUniqueNetIdWrapper& DoneID : ToRemove)
			{
				PendingUnregisterIDs.Remove(DoneID);
			}
		}
		else
		{
			// 인터페이스가 없으면 이미 정리된 것으로 간주
			PendingUnregisterIDs.Empty();
		}*/

		// 2. 컴포넌트 및 OSS 상태 최종 확인
		if (IsVoiceFullyCleanedUp())
		{
			bIsCheckingVoiceCleanup = false;
			Server_NotifyReadyForTravel();
			AO_LOG(LogJM, Log, TEXT("All Voice Resources Cleaned Up."));
		}
		
		/*
		bool bAnyComponentRemaining = false;
		UWorld* World = GetWorld();

		// 1. VoipListenerSynthComponent가 월드에 남아있는지 확인
		for (TObjectIterator<UVoipListenerSynthComponent> It; It; ++It)
		{
			if (It->GetWorld() == World && IsValid(*It))
			{
				bAnyComponentRemaining = true;
				break;
			}
		}

		// 2. VOIPTalker가 월드에 남아있는지 확인
		if (!bAnyComponentRemaining)
		{
			for (TObjectIterator<UVOIPTalker> It; It; ++It)
			{
				if (It->GetWorld() == World && IsValid(*It))
				{
					bAnyComponentRemaining = true;
					break;
				}
			}
		}

		// 모든 관련 컴포넌트가 사라졌다면 서버에 보고
		if (!bAnyComponentRemaining)
		{
			bIsCheckingVoiceCleanup = false;
			Server_NotifyReadyForTravel();
			AO_LOG(LogJM, Log, TEXT("Voice Cleanup Complete - Reporting to Server"));
		}*/
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

	/*// 1. 현재 존재하는 모든 원격 타커의 ID를 수집
	PendingUnregisterIDs.Empty();
	if (UWorld* World = GetWorld())
	{
		if (AGameStateBase* GS = World->GetGameState())
		{
			for (APlayerState* PS : GS->PlayerArray)
			{
				if (PS && (PS != PlayerState))	// 본인 PS 가 아니라면 추가
				// if (PS && !PS-> ->IsLocalPlayerController())
				{
					// 유효한 NetId가 있는 경우만 추가
					if (PS->GetUniqueId().IsValid())
					{
						PendingUnregisterIDs.Add(PS->GetUniqueId());
					}
				}
			}
		}
	}*/
	
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

	/*// 1. VOIP 관련 Synth 컴포넌트 정리
	for (TObjectIterator<UVoipListenerSynthComponent> It; It; ++It)
	{
		if (IsValid(*It) && It->IsRegistered())
		{
			// It->Stop();	// 컴파일 에러 
			It->UnregisterComponent();
			It->MarkAsGarbage();
		}
	}

	// 2. 오디오 컴포넌트 정리 (특히 VOIP 내부 컴포넌트 타겟팅)
	for (TObjectIterator<UAudioComponent> It; It; ++It)
	{
		if (IsValid(*It) && It->IsRegistered())
		{
			// 이름에 "Voip"가 포함되어 있거나, 부모(Outer)가 Voip 관련 객체인 경우
			bool bIsVoipRelated = It->GetName().Contains(TEXT("Voip"));
            
			if (!bIsVoipRelated && It->GetOuter())
			{
				bIsVoipRelated = It->GetOuter()->IsA<UVoipListenerSynthComponent>() || 
								 It->GetOuter()->GetName().Contains(TEXT("Voip"));
			}

			if (bIsVoipRelated)
			{
				It->Stop();
				It->UnregisterComponent(); // 핵심: 렌더링 씬에서 즉시 제거
				It->MarkAsGarbage();
				AO_LOG(LogJM, Log, TEXT("Cleanup: Forced Unregister AudioComponent - %s"), *It->GetName());
			}
		}
	}*/

	// 3. OnlineSubsystem 소리 정지
	if (UAO_OnlineSessionSubsystem* OSS = GetGameInstance()->GetSubsystem<UAO_OnlineSessionSubsystem>())
	{
		OSS->MuteAllRemoteTalker();	// JM : 안정성을 위해 Mute도 해주기
		OSS->StopVoiceChat();
	}
	else
	{
		AO_ENSURE(false, TEXT("Can't Get OSS"));
	}

	/*
	for (TObjectIterator<UVoipListenerSynthComponent> It; It; ++It)
	{
		if (!IsValid(*It))
		{
			continue;
		}

		if (It->IsRegistered())
		{
			It->ClosePacketStream();
			It->Deactivate();
			It->UnregisterComponent();
			It->MarkAsGarbage();
			AO_LOG(LogJM, Log, TEXT("Cleanup: Forced Unregister SynthComponent"));
			// It->DestroyComponent();
		}
	}

	for (TObjectIterator<UAudioComponent> It; It; ++It)
	{
		if (IsValid(*It) && It->IsRegistered())
		{
			if (It->GetName().Contains(TEXT("Voip")))
			{
				It->Stop();
				It->UnregisterComponent();
				It->MarkAsGarbage();
				AO_LOG(LogJM, Log, TEXT("Cleanup: Forced Unregister UAudioComponent"));
				// It->DestroyComponent();
			}
		}
	}
	*/

	
	
	/*
	// 1. 모든 오디오/보이스 관련 컴포넌트를 전수 조사 (월드 제한 해제)
	// TObjectIterator는 Transient 패키지 내의 객체도 모두 찾아냅니다.
	// A. VoipListenerSynthComponent 정리
	for (TObjectIterator<UVoipListenerSynthComponent> It; It; ++It)
	{
		// 유효하지 않거나 이미 파괴 중인 객체 제외
		if (!IsValid(*It)) continue;

		// [중요] 월드 체크를 하지 않거나, 등록된 상태(IsRegistered)라면 무조건 해제
		if (It->IsRegistered() || It->IsActive())
		{
			AO_LOG(LogJM, Warning, TEXT("Cleaning Transient UVoipListenerSynthComponent : %s"), *It->GetName());
            
			It->ClosePacketStream();
			// It->Stop();
			It->Deactivate();
            
			// 렌더링 씬에서 강제로 등록 해제 (로그의 에러를 해결하는 핵심)
			It->UnregisterComponent(); 
            
			// Transient에 있는 경우 MarkAsGarbage를 통해 GC 대상임을 명시
			
			// It->MarkAsGarbage();
			// It->BeginDestroy();
			It->DestroyComponent();
		}
	}

	// B. 일반 AudioComponent 중 VOIP용 사운드 시트를 사용하는 것들 정리
	for (TObjectIterator<UAudioComponent> It; It; ++It)
	{
		if (!IsValid(*It)) continue;

		// 로그에 나온 것처럼 Transient에 있으면서 문제를 일으키는 것들 타겟팅
		if (It->IsRegistered())
		{
			// 이름이나 아우터(Outer)에 Voip가 포함되어 있다면 보이스 관련임
			if (It->GetName().Contains(TEXT("Voip")) || (It->GetOuter() && It->GetOuter()->IsA<UVoipListenerSynthComponent>()))
			{
				AO_LOG(LogJM, Warning, TEXT("Cleaning Orphaned Audio Component: %s"), *It->GetPathName());
				It->Stop();
				It->UnregisterComponent();
				// It->MarkAsGarbage();
				// It->BeginDestroy(); // 이건 외부에서 호출하면 안되는구나?! crash 나네?
				It->DestroyComponent();
			}
		}
	}

	for (TObjectIterator<UVOIPTalker> It; It; ++It)		// VOIPTalker를 모두 찾아서 제거
	{
		if (!IsValid(*It)) continue;

		It->Deactivate();
		It->UnregisterComponent();
		It->DestroyComponent();
		AO_LOG(LogJM, Log, TEXT("Cleanup: Forced Destruction of VOIPTalker"));
	}
	*/

	/*// 2. 엔진 레벨의 오디오 하드웨어 중단 (이전 답변 드린 내용 포함)
	if (GEngine && GEngine->GetMainAudioDevice())
	{
		GEngine->GetMainAudioDevice()->StopAllSounds(true);
	}*/
	

	
	/* 내가 수동으로 지우는건 안좋은 거 같은데... */
	// 월드 뿐만 아니라 Transient 도 확인해서 지워보자
	/*if (UWorld* World = GetWorld())
	{
		// 2. 모든 SynthComponent 및 AudioComponent 중지 및 해제
		// TObjectIterator는 월드에 상관없이 모든 객체를 찾으므로 월드 체크가 필수입니다. // 강화
		for (TObjectIterator<UAudioComponent> It; It; ++It)
		{
			if (It->GetWorld() == World)
			{
				It->Stop();
				It->SetComponentTickEnabled(false);
				It->UnregisterComponent();
				It->DestroyComponent(); // GC로는 안되는데? 여전히 크래시가발생해 // Unregister만으로도 충분히 안전하며 GC가 처리하게 둠
				AO_LOG(LogJM, Log, TEXT("Cleanup: Forced Unregister & Destroy Audio Component"));
			}
		}
		
		// JM : 실제 에러의 주범인 'VoipListenerSynthComponent'를 모두 찾아 제거
		for (TObjectIterator<UVoipListenerSynthComponent> It; It; ++It)		// TObjectIterator는 Transient 패키지에 숨은 컴포넌트까지 다 찾아냄
		{
			if (It->GetWorld() == World)	// 이 컴포넌트가 현재 파괴되려는 월드와 연결되어 있는지 확인
			{
				It->ClosePacketStream();	// 강화
				It->BeginDestroy();			// 
				
				// It->Stop();					// 강화 (사용 불가 컴파일에러)
				It->Deactivate();			// 강화
				//	It->GetAudioComponent()->UnregisterComponent(); // 안됨 (컴파일에러)
				It->UnregisterComponent();
				It->DestroyComponent();		// 강제 삭제까지
				AO_LOG(LogJM, Log, TEXT("Cleanup: Forced Unregister SynthComponent"));
			}
		}

		for (TObjectIterator<UVOIPTalker> It; It; ++It)		// VOIPTalker를 모두 찾아서 제거
		{
			if (It->GetWorld() == World)
			{
				It->Deactivate();
				It->UnregisterComponent();
				It->DestroyComponent();
				AO_LOG(LogJM, Log, TEXT("Cleanup: Forced Destruction of VOIPTalker"));
			}
		}
	}*/
	
	
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

void AAO_PlayerController_InGameBase::InitCameraManager()
{
	checkf(CameraManagerComponent, TEXT("CameraManagerComponent not found"));
	
	AAO_PlayerCharacter* PlayerCharacter = Cast<AAO_PlayerCharacter>(GetPawn());
	checkf(PlayerCharacter, TEXT("Character not found"));

	CameraManagerComponent->BindCameraComponents(PlayerCharacter->GetSpringArm(), PlayerCharacter->GetCamera());
	CameraManagerComponent->PushCameraState(FGameplayTag::RequestGameplayTag(FName("Camera.Default")));

	if (UAbilitySystemComponent* ASC = PlayerCharacter->GetAbilitySystemComponent())
	{
		CameraManagerComponent->BindToASC(ASC);
	}
}

bool AAO_PlayerController_InGameBase::IsVoiceFullyCleanedUp()
{
	AGameStateBase* GS_Base = GetWorld()->GetGameState<AGameStateBase>();
	if (GS_Base)
	{
		UAO_OnlineSessionSubsystem* OSS = GetGameInstance()->GetSubsystem<UAO_OnlineSessionSubsystem>();
		if (OSS)
		{
			IOnlineVoicePtr VoiceInterface = OSS->GetOnlineVoiceInterface();
			if (VoiceInterface.IsValid())
			{
				for (APlayerState* PS : GS_Base->PlayerArray)
				{
					if (!AO_ENSURE(PS, TEXT("PS is Null")))
					{
						continue;
					}

					AAO_PlayerState* AO_PS = Cast<AAO_PlayerState>(PS);
					if (!AO_ENSURE(AO_PS, TEXT("Cast Failed PS -> AO_PS")))
					{
						continue;
					}

					TSharedPtr<const FUniqueNetId> PlayerID = AO_PS->GetUniqueId().GetUniqueNetId();
					if (PlayerID)
					{
						if (PlayerID == PlayerState->GetUniqueId().GetUniqueNetId())
						{
							/*if (VoiceInterface->UnregisterLocalTalker(0))
							{
								AO_LOG(LogJM, Warning, TEXT("자신을 아직 Unregister 하지 못한 경우"));
								return false;
							}*/
							continue;
						}
						
						if (!VoiceInterface->IsMuted(0, *PlayerID))
						{
							AO_LOG(LogJM, Warning, TEXT("IsVoiceFullyCleanedUp: Player %s is not muted"), *PlayerID->ToString());
							// VoiceInterface->MuteRemoteTalker(0, *PlayerID, false); // 이거로 해결 안됨
							return false;
						}
						
					}
				}
			}
		}
	}

	// TODO: 이게 필요한가? 안도는데?
	// 2. 엔진이 내부적으로 Audio 자원을 씬에서 뺐는지 확인 (이게 핵심)
	for (TObjectIterator<UAudioComponent> It; It; ++It)
	{
		AO_LOG(LogJM, Warning, TEXT("Audio Component가 있다 : %s"), *It->GetName());
		// 이름에 "Voip"가 들어간 녀석이 '아직' Registered 상태라면 엔진이 작업 중인 것임
		if (It->IsRegistered() && It->GetName().Contains(TEXT("Voip")))
		{
			AO_LOG(LogJM, Warning, TEXT("엔진이 아직 VOIP 컴포넌트를 정리 중입니다... Wait: %s"), *It->GetName());
			It->UnregisterComponent();
			return false; 
		}
	}

	for (TObjectIterator<UVoipListenerSynthComponent> It; It; ++It)
	{
		// IsValid(GC 대상이 아님)이고 IsRegistered(씬에 등록됨) 상태라면 아직 정리 안 됨
		// 이걸 UnregisterRemoteTalker보다 먼저 하면 안됨
		if (IsValid(*It) && It->IsRegistered())
		{
			AO_LOG(LogJM, Warning, TEXT("IsVoiceFullyCleanedUp: UVoipListenerSynthComponent is still Registered - %s"), *It->GetName());
			It->UnregisterComponent();
			return false;
		}
	}

	AO_LOG(LogJM, Log, TEXT("End"));
	return true;

	/*
	// 1. VOIP 리스너 컴포넌트 체크
	for (TObjectIterator<UVoipListenerSynthComponent> It; It; ++It)
	{
		// IsValid(GC 대상이 아님)이고 IsRegistered(씬에 등록됨) 상태라면 아직 정리 안 됨
		if (IsValid(*It) && It->IsRegistered())
		{
			AO_LOG(LogJM, Warning, TEXT("IsVoiceFullyCleanedUp: UVoipListenerSynthComponent is still Registered - %s"), *It->GetName());
			return false;
		}
	}

	// 2. VOIP 관련 오디오 컴포넌트 체크 (핵심)
	// 크래시 로그에서 AudioComponent가 문제를 일으킨 것으로 확인됨 
	for (TObjectIterator<UAudioComponent> It; It; ++It)
	{
		if (IsValid(*It) && It->IsRegistered())
		{
			// 이름에 "Voip"가 포함되어 있거나, 부모(Outer)가 Voip 관련 객체인 경우 체크
			bool bIsVoipRelated = It->GetName().Contains(TEXT("Voip"));
            
			if (!bIsVoipRelated && It->GetOuter())
			{
				bIsVoipRelated = It->GetOuter()->IsA<UVoipListenerSynthComponent>() || 
								 It->GetOuter()->GetName().Contains(TEXT("Voip"));
			}

			if (bIsVoipRelated)
			{
				AO_LOG(LogJM, Warning, TEXT("IsVoiceFullyCleanedUp: VOIP AudioComponent is still Registered - %s"), *It->GetName());
				return false;
			}
		}
	}

	// 3. (선택사항) VOIP 토커 체크
	for (TObjectIterator<UVOIPTalker> It; It; ++It)
	{
		if (IsValid(*It) && !It->IsGarbageEliminationEnabled()) // UVOIPTalker는 컴포넌트가 아니므로 IsRegistered 없음
		{
			// Talker가 여전히 유효하다면 완전히 정리된 것이 아닐 수 있음
			// 필요에 따라 추가 로직 작성
			It->UnregisterComponent();
			It->MarkAsGarbage();
			AO_LOG(LogJM, Warning, TEXT("IsVoiceFullyCleanedUp: UVOIPTalker is still Registered - %s"), *It->GetName());
			return false;
		}
	}
	*/
	

	/*for (TObjectIterator<UVoipListenerSynthComponent> It; It; ++It)
	{
		if (IsValid(*It) && It->IsRegistered())
		{
			AO_LOG(LogJM, Warning, TEXT("UVoipListenerSynthComponent is Registered"));
			return false;
		}
	}*/



	

	// A. OSS 타커 리스트가 비었는가?
	/*if (PendingUnregisterIDs.Num() > 0)
	{
		AO_LOG(LogJM, Warning, TEXT("PendingUnregisterIDs.Num() : %d > 0"), PendingUnregisterIDs.Num());
		return false;
	}*/

	/*
	// B. 월드에 잔류 컴포넌트가 있는가?
	UWorld* World = GetWorld();
	
	// B. 월드에 살아있는 Audio/Synth Component가 하나라도 있는가?
	for (TObjectIterator<UAudioComponent> It; It; ++It)
	{
		// IsRegistered()가 true이면서 이 월드 소속인 것이 있다면 아직 정리 중인 것임
		if (It->IsRegistered() || IsValid(*It))
		{
			AO_LOG(LogJM, Warning, TEXT("AudioComp 남아있음"));
			return false; 
		}
	}
	
	for (TObjectIterator<UVoipListenerSynthComponent> It; It; ++It)
	{
		if (IsValid(*It))
		// if (It->GetWorld() == World && (IsValid(*It) || It->IsGarbageEliminationEnabled()))
		// if (It->GetWorld() == World && !It->IsGarbageEliminationEnabled())
		{
			AO_LOG(LogJM, Warning, TEXT("VoipListenerSynthComp 남아있음"));
			return false;
		}
	}
	for (TObjectIterator<UVOIPTalker> It; It; ++It)
	{
		if (IsValid(*It))
		// if (It->GetWorld() == World && (IsValid(*It) || It->IsGarbageEliminationEnabled()))
		// if (It->GetWorld() == World && !It->IsGarbageEliminationEnabled())
		{
			AO_LOG(LogJM, Warning, TEXT("VOIP Talker 남아있음"));
			return false;
		}
	}

	// 월드 필터 없이 IsRegistered 상태인 Voip 컴포넌트가 하나라도 있으면 false
	for (TObjectIterator<UVoipListenerSynthComponent> It; It; ++It)
	{
		if (IsValid(*It) && It->IsRegistered())
		{
			AO_LOG(LogJM, Warning, TEXT("Transient에 VoipListenerSynthComp 남아있음"));
			return false;
		}
	}
	
	AO_LOG(LogJM, Log, TEXT("End"));
	return true;

	*/
}
