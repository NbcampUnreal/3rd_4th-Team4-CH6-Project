// AO_GameUserSettings.cpp (장주만)
#include "Settings/AO_GameUserSettings.h"
#include "AO_Log.h"

UAO_GameUserSettings* UAO_GameUserSettings::GetGameUserSettings()
{
	return Cast<UAO_GameUserSettings>(UGameUserSettings::GetGameUserSettings());
}

void UAO_GameUserSettings::ApplyCustomSettings()
{
}
