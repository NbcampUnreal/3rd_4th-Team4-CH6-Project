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
