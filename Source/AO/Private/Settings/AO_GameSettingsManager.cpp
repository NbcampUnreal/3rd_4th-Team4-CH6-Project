// AO_GameSettingsManager.cpp (장주만)


#include "Settings/AO_GameSettingsManager.h"

#include "AO_Log.h"

void UAO_GameSettingsManager::Initialize(FSubsystemCollectionBase& Collection)
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	Super::Initialize(Collection);

	if (UAO_GameUserSettings* Settings = GetGameUserSettings())
	{
		Settings->ApplySettings(false);		// 엔진 기본 설정 적용 (그래픽, 해상도 등)
		Settings->ApplyCustomSettings();								// 사용자 정의 설정 적용 (볼륨, 마우스 감도 등)
	}
	else
	{
		AO_LOG(LogJM, Warning, TEXT("Failed to get game user settings."));
		return;
	}
	AO_LOG(LogJM, Log, TEXT("End"));
}

void UAO_GameSettingsManager::Deinitialize()
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	Super::Deinitialize();
	AO_LOG(LogJM, Log, TEXT("End"));
}

UAO_GameUserSettings* UAO_GameSettingsManager::GetGameUserSettings() const
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	AO_LOG(LogJM, Log, TEXT("End"));
	return UAO_GameUserSettings::GetGameUserSettings();
}

void UAO_GameSettingsManager::ApplyAndSaveAllSettings()
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	if (UAO_GameUserSettings* Settings = GetGameUserSettings())
	{
		Settings->ApplySettings(false);	// 변경된 설정 값 반영
		Settings->ApplyCustomSettings();							// 커스텀 설정 적용 (볼륨, 감도 등)
		OnSettingsApplied.Broadcast();								
		AO_LOG(LogJM, Log, TEXT("All Settings Applied and Saved"));
	}
	else
	{
		AO_LOG(LogJM, Warning, TEXT("Failed to get game user settings."));
	}
	AO_LOG(LogJM, Log, TEXT("End"));
}

/*
void UAO_GameSettingsManager::RevertAndApplyDefaultSettings()
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	if (UAO_GameUserSettings* Settings = GetGameUserSettings())
	{
		Settings->LoadSettings();		// 이스크에 저장된 마지막 설정값으로 되돌림
		// Settings->SetToDefaults();		// 만약 완전한 초기화를 원한다면 실행 (TODO: 이건 따로 뺴야겠군)
		ApplyAndSaveAllSettings();		// 적용 및 저장
		AO_LOG(LogJM, Log, TEXT("Settings reverted to last saved state and Applied"));
	}
	else
	{
		AO_LOG(LogJM, Warning, TEXT("Failed to get game user settings."));
	}
	AO_LOG(LogJM, Log, TEXT("End"));
}*/

void UAO_GameSettingsManager::ApplyResolutionSettings()
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	if (UAO_GameUserSettings* Settings = GetGameUserSettings())
	{
		Settings->ApplyResolutionSettings(false);
		Settings->RequestUIUpdate();			// ApplySettings 에서 호출되는 내용
		Settings->SaveSettings();				// 분리하다보니 따로 호출함
		OnSettingsApplied.Broadcast();								// 설적 적용 완료 이벤트를 발생시켜 알림
		AO_LOG(LogJM, Log, TEXT("Resolution Settings Applied and Saved"));
	}
	else
	{
		AO_LOG(LogJM, Warning, TEXT("Failed to get game user settings."));
	}
	AO_LOG(LogJM, Log, TEXT("End"));
}

void UAO_GameSettingsManager::ApplyNonResolutionSettings()
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	if (UAO_GameUserSettings* Settings = GetGameUserSettings())
	{
		Settings->ApplyNonResolutionSettings();
		Settings->RequestUIUpdate();			// ApplySettings 에서 호출되는 내용
		Settings->SaveSettings();				// 분리하다보니 따로 호출함
		Settings->ApplyCustomSettings();							// 커스텀 설정 적용 (볼륨, 감도 등)
		OnSettingsApplied.Broadcast();								// 설적 적용 완료 이벤트를 발생시켜 알림
		AO_LOG(LogJM, Log, TEXT("NonResolution Settings Applied and Saved"));
	}
	else
	{
		AO_LOG(LogJM, Warning, TEXT("Failed to get game user settings."));
	}
	AO_LOG(LogJM, Log, TEXT("End"));
}

void UAO_GameSettingsManager::SetOverallScalability(EScalabilityLevel NewLevel)
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	if (UAO_GameUserSettings* Settings = GetGameUserSettings())
	{
		Settings->SetOverallScalabilityLevel(static_cast<int32>(NewLevel));
	}
	else
	{
		AO_LOG(LogJM, Warning, TEXT("Failed to get game user settings."));
	}
	AO_LOG(LogJM, Log, TEXT("End"));
}

void UAO_GameSettingsManager::SetAntiAliasingScalability(EScalabilityLevel NewLevel)
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	if (UAO_GameUserSettings* Settings = GetGameUserSettings())
	{
		Settings->SetAntiAliasingQuality(static_cast<int32>(NewLevel));
	}
	else
	{
		AO_LOG(LogJM, Warning, TEXT("Failed to get game user settings."));
	}
	AO_LOG(LogJM, Log, TEXT("End"));
}

void UAO_GameSettingsManager::SetViewDistanceScalability(EScalabilityLevel NewLevel)
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	if (UAO_GameUserSettings* Settings = GetGameUserSettings())
	{
		Settings->SetViewDistanceQuality(static_cast<int32>(NewLevel));
	}
	else
	{
		AO_LOG(LogJM, Warning, TEXT("Failed to get game user settings."));
	}
	AO_LOG(LogJM, Log, TEXT("End"));
}

void UAO_GameSettingsManager::SetShadowScalability(EScalabilityLevel NewLevel)
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	if (UAO_GameUserSettings* Settings = GetGameUserSettings())
	{
		Settings->SetShadowQuality(static_cast<int32>(NewLevel));
	}
	else
	{
		AO_LOG(LogJM, Warning, TEXT("Failed to get game user settings."));
	}
	AO_LOG(LogJM, Log, TEXT("End"));
}

void UAO_GameSettingsManager::SetGlobalIlluminationScalability(EScalabilityLevel NewLevel)
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	if (UAO_GameUserSettings* Settings = GetGameUserSettings())
	{
		Settings->SetGlobalIlluminationQuality(static_cast<int32>(NewLevel));
	}
	else
	{
		AO_LOG(LogJM, Warning, TEXT("Failed to get game user settings."));
	}
	AO_LOG(LogJM, Log, TEXT("End"));
}

void UAO_GameSettingsManager::SetReflectionScalability(EScalabilityLevel NewLevel)
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	if (UAO_GameUserSettings* Settings = GetGameUserSettings())
	{
		Settings->SetReflectionQuality(static_cast<int32>(NewLevel));
	}
	else
	{
		AO_LOG(LogJM, Warning, TEXT("Failed to get game user settings."));
	}
	AO_LOG(LogJM, Log, TEXT("End"));
}

void UAO_GameSettingsManager::SetTextureScalability(EScalabilityLevel NewLevel)
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	if (UAO_GameUserSettings* Settings = GetGameUserSettings())
	{
		Settings->SetTextureQuality(static_cast<int32>(NewLevel));
	}
	else
	{
		AO_LOG(LogJM, Warning, TEXT("Failed to get game user settings."));
	}
	AO_LOG(LogJM, Log, TEXT("End"));
}

void UAO_GameSettingsManager::SetVisualEffectScalability(EScalabilityLevel NewLevel)
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	if (UAO_GameUserSettings* Settings = GetGameUserSettings())
	{
		Settings->SetVisualEffectQuality(static_cast<int32>(NewLevel));
	}
	else
	{
		AO_LOG(LogJM, Warning, TEXT("Failed to get game user settings."));
	}
	AO_LOG(LogJM, Log, TEXT("End"));
}

void UAO_GameSettingsManager::SetPostProcessingScalability(EScalabilityLevel NewLevel)
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	if (UAO_GameUserSettings* Settings = GetGameUserSettings())
	{
		Settings->SetPostProcessingQuality(static_cast<int32>(NewLevel));
	}
	else
	{
		AO_LOG(LogJM, Warning, TEXT("Failed to get game user settings."));
	}
	AO_LOG(LogJM, Log, TEXT("End"));
}

void UAO_GameSettingsManager::SetFoliageScalability(EScalabilityLevel NewLevel)
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	if (UAO_GameUserSettings* Settings = GetGameUserSettings())
	{
		Settings->SetFoliageQuality(static_cast<int32>(NewLevel));
	}
	else
	{
		AO_LOG(LogJM, Warning, TEXT("Failed to get game user settings."));
	}
	AO_LOG(LogJM, Log, TEXT("End"));
}

void UAO_GameSettingsManager::SetShadingScalability(EScalabilityLevel NewLevel)
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	if (UAO_GameUserSettings* Settings = GetGameUserSettings())
	{
		Settings->SetShadingQuality(static_cast<int32>(NewLevel));
	}
	else
	{
		AO_LOG(LogJM, Warning, TEXT("Failed to get game user settings."));
	}
	AO_LOG(LogJM, Log, TEXT("End"));
}

void UAO_GameSettingsManager::SetVSyncEnabled(bool bEnable)
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	if (UAO_GameUserSettings* Settings = GetGameUserSettings())
	{
		Settings->SetVSyncEnabled(bEnable);
	}
	else
	{
		AO_LOG(LogJM, Warning, TEXT("Failed to get game user settings."));
	}
	AO_LOG(LogJM, Log, TEXT("End"));
}

void UAO_GameSettingsManager::SetFullscreenMode(EWindowMode::Type InFullscreenMode)
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	if (UAO_GameUserSettings* Settings = GetGameUserSettings())
	{
		Settings->SetFullscreenMode(InFullscreenMode);
	}
	else
	{
		AO_LOG(LogJM, Warning, TEXT("Failed to get game user settings."));
	}
	AO_LOG(LogJM, Log, TEXT("End"));
}


void UAO_GameSettingsManager::SetMasterVolume(float NewVolume)
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	if (UAO_GameUserSettings* Settings = GetGameUserSettings())
	{
		Settings->MasterVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);		// TODO: 향후 Magic Number 수정필요
	}
	else
	{
		AO_LOG(LogJM, Warning, TEXT("Failed to get game user settings."));
	}
	AO_LOG(LogJM, Log, TEXT("End"));
}

void UAO_GameSettingsManager::SetMouseSensitivity(float NewSensitivity)
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	if (UAO_GameUserSettings* Settings = GetGameUserSettings())
	{
		Settings->MouseSensitivity = FMath::Clamp(NewSensitivity, 0.1f, 5.0f);	// TODO: 향후 Magic Number 수정 필요
	}
	else
	{
		AO_LOG(LogJM, Warning, TEXT("Failed to get game user settings."));
	}
	AO_LOG(LogJM, Log, TEXT("End"));
}
