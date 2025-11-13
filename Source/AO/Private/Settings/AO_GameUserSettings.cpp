// AO_GameUserSettings.cpp (장주만)
#include "Settings/AO_GameUserSettings.h"
#include "AO_Log.h"

UAO_GameUserSettings* UAO_GameUserSettings::GetGameUserSettings()
{
	return Cast<UAO_GameUserSettings>(UGameUserSettings::GetGameUserSettings());
}

void UAO_GameUserSettings::ApplyCustomSettings()
{
	AO_LOG(LogJM, Log, TEXT("Start (MasterVolume : %f, MusicVolume : %f, SFXVolume : %f, MouseSensitivity : %f, bInvertYAxis : %d, GameLanguage : %s"),
		MasterVolume, MusicVolume, SFXVolume, MouseSensitivity, bInvertYAxis, *GameLanguage);

	
	
	AO_LOG(LogJM, Log, TEXT("End"));
}
