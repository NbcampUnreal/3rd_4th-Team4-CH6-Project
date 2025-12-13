// AO_PlayerController.cpp (장주만)


#include "Player/PlayerController/AO_PlayerController.h"

#include "AO_Log.h"
#include "CommonLoadingScreen/Public/LoadingScreenManager.h"

void AAO_PlayerController::PreClientTravel(const FString& PendingURL, ETravelType TravelType, bool bIsSeamlessTravel)
{
	Super::PreClientTravel(PendingURL, TravelType, bIsSeamlessTravel);
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow, FString::Printf(TEXT("Client Travel To %s"), *PendingURL), false);	// JM : 디버그용

	ULoadingScreenManager* LSM = GetGameInstance()->GetSubsystem<ULoadingScreenManager>();
	if (LSM)
	{
		LSM->PendingMapName = PendingURL;
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow, FString::Printf(TEXT("Update PendingMapName To %s"), *LSM->PendingMapName), false);	// JM : 디버그용
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow, FString::Printf(TEXT("no lsm")), false);	// JM : 디버그용
	}
}

void AAO_PlayerController::CreateSettingsWidgetInstance(const int32 ZOrder, const ESlateVisibility Visibility)
{
	AO_LOG(LogJM, Log, TEXT("Start"));

	if (!AO_ENSURE(!SettingsWidgetInstance, TEXT("Settings Widget Instance is Already Created")))	// 위젯 중복 생성 방지
	{
		return;
	}

	if (!AO_ENSURE(SettingsWidgetClass, TEXT("Settings Widget Class is nullptr")))
	{
		return;
	}

	SettingsWidgetInstance = CreateWidget<UAO_UserWidget>(this, SettingsWidgetClass);
	if (!AO_ENSURE(SettingsWidgetInstance, TEXT("Settings Widget Instance Create Failed")))
	{
		return;
	}

	SettingsWidgetInstance->AddToViewport(ZOrder);
	SettingsWidgetInstance->SetVisibility(Visibility);
	
	AO_LOG(LogJM, Log, TEXT("End"));
}
