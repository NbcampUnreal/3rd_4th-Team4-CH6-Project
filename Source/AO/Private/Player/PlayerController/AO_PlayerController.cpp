// AO_PlayerController.cpp (장주만)


#include "Player/PlayerController/AO_PlayerController.h"

#include "AO_Log.h"
#include "CommonLoadingScreen/Public/LoadingScreenManager.h"
#include "Online/AO_OnlineSessionSubsystem.h"
#include "Net/VoiceConfig.h"
#include "VoipListenerSynthComponent.h"

void AAO_PlayerController::PreClientTravel(const FString& PendingURL, ETravelType TravelType, bool bIsSeamlessTravel)
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow, FString::Printf(TEXT("Client Travel To %s"), *PendingURL), false);	// JM : 디버그용
	
	if (IsLocalController())
	{
		CleanupAudioResource();
		UpdateLoadingMapName(PendingURL);
	}

	Super::PreClientTravel(PendingURL, TravelType, bIsSeamlessTravel);
	AO_LOG(LogJM, Log, TEXT("End"));
}

UAO_UserWidget* AAO_PlayerController::GetOrCreateSettingsWidgetInstance()
{
	if (SettingsWidgetInstance)
	{
		return SettingsWidgetInstance;
	}

	if (!AO_ENSURE(SettingsWidgetClass, TEXT("Settings Widget Class is nullptr")))
	{
		return nullptr;
	}

	SettingsWidgetInstance = CreateWidget<UAO_UserWidget>(this, SettingsWidgetClass);
	if (!AO_ENSURE(SettingsWidgetInstance, TEXT("Settings Widget Instance Create Failed")))
	{
		return nullptr;
	}

	// UIStackManager가 AddToViewport/RemoveFromParent를 담당
	return SettingsWidgetInstance;
}

void AAO_PlayerController::CleanupAudioResource()
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

	if (UWorld* World = GetWorld())
	{
		// JM : 실제 에러의 주범인 'VoipListenerSynthComponent'를 모두 찾아 제거
		for (TObjectIterator<UVoipListenerSynthComponent> It; It; ++It)		// TObjectIterator는 Transient 패키지에 숨은 컴포넌트까지 다 찾아냄
		{
			if (It->GetWorld() == World)	// 이 컴포넌트가 현재 파괴되려는 월드와 연결되어 있는지 확인
			{
				It->UnregisterComponent();
				It->DestroyComponent();
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
	}
	
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_PlayerController::UpdateLoadingMapName(const FString& PendingURL) const
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	
	ULoadingScreenManager* LSM = GetGameInstance()->GetSubsystem<ULoadingScreenManager>();
	if (LSM)
	{
		LSM->PendingMapName = PendingURL;
	}
	else
	{
		AO_ENSURE(false, TEXT("Loading Screen Manager is nullptr"));
	}
	
	AO_LOG(LogJM, Log, TEXT("End"));
}
