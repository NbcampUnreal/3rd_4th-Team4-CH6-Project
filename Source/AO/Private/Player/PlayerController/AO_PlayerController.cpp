// AO_PlayerController.cpp (장주만)


#include "Player/PlayerController/AO_PlayerController.h"

#include "AO_Log.h"
#include "CommonLoadingScreen/Public/LoadingScreenManager.h"

void AAO_PlayerController::PreClientTravel(const FString& PendingURL, ETravelType TravelType, bool bIsSeamlessTravel)
{
	Super::PreClientTravel(PendingURL, TravelType, bIsSeamlessTravel);
	// GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow, FString::Printf(TEXT("Client Travel To %s"), *PendingURL), false);	// JM : 디버그용

	ULoadingScreenManager* LSM = GetGameInstance()->GetSubsystem<ULoadingScreenManager>();
	if (!AO_ENSURE(LSM, TEXT("Loading Screen Manager is nullptr")))
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow, FString::Printf(TEXT("no lsm")), false);	// JM : 디버그용
		return;
	}

	LSM->PendingMapName = PendingURL;
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
