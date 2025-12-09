// AO_PlayerController.cpp (장주만)


#include "Player/PlayerController/AO_PlayerController.h"

#include "AO_Log.h"

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
