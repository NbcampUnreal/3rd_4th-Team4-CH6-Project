// AO_PlayerController.cpp (장주만)


#include "Player/PlayerController/AO_PlayerController.h"

#include "AO_Log.h"

void AAO_PlayerController::CreateSettingsWidgetInstance(const int32 ZOrder, const ESlateVisibility Visibility)
{
	AO_LOG(LogJM, Log, TEXT("Start"));

	if (SettingsWidgetInstance)		// 위젯 중복 생성 방지
	{
		AO_LOG(LogJM, Warning, TEXT("Settings Widget Instance is Already Created"));
		return;
	}

	if (!SettingsWidgetClass)
	{
		AO_LOG(LogJM, Warning, TEXT("Settings Widget Class is nullptr"));
		return;
	}

	SettingsWidgetInstance = CreateWidget<UAO_UserWidget>(this, SettingsWidgetClass);
	if (!SettingsWidgetInstance)
	{
		AO_LOG(LogJM, Warning, TEXT("Settings Widget Instance is nullptr"));
		return;
	}

	SettingsWidgetInstance->AddToViewport(ZOrder);
	SettingsWidgetInstance->SetVisibility(Visibility);
	
	AO_LOG(LogJM, Log, TEXT("End"));
}
