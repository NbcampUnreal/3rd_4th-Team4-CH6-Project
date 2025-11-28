// AO_GameSettingsManager.cpp (장주만)


#include "Settings/AO_GameSettingsManager.h"

#include "AO_DelegateManager.h"
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

void UAO_GameSettingsManager::SetToDefaults()
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	if (UAO_GameUserSettings* Settings = GetGameUserSettings())
	{
		Settings->SetToDefaults();
		if (UAO_DelegateManager* DelegateManager = GetGameInstance()->GetSubsystem<UAO_DelegateManager>())
		{
			DelegateManager->OnResetAllSettings.Broadcast();
			AO_LOG(LogJM, Log, TEXT("Broadcast DelegateManager::OnResetAllSettings"));
		}
		else{
			AO_LOG(LogJM, Warning, TEXT("Failed to get DelegateManager"));
		}
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

void UAO_GameSettingsManager::SetScreenResolutionByIndex(int32 ResolutionIndex)
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	const TArray<FResolutionInfo>& ResolutionInfoList = GetResolutionInfoList();

	if (ResolutionInfoList.IsValidIndex(ResolutionIndex))
	{
		if (UAO_GameUserSettings* Settings = GetGameUserSettings())
		{
			Settings->SetScreenResolution(ResolutionInfoList[ResolutionIndex].Resolution);
		}
		else
		{
			AO_LOG(LogJM, Warning, TEXT("Failed to get game user settings."));
		}
	}
	else
	{
		AO_LOG(LogJM, Warning, TEXT("Resolution Index is not valid"));
	}
	AO_LOG(LogJM, Log, TEXT("End"));
}

TArray<FResolutionInfo> UAO_GameSettingsManager::GetSupportedScreenResolutionInfos() const
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	AO_LOG(LogJM, Log, TEXT("End"));
	return GetResolutionInfoList();
}

int32 UAO_GameSettingsManager::GetCurrentResolutionIndex() const
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	const FIntPoint CurrentResolution = GetAppliedScreenResolution();
	const TArray<FResolutionInfo>& ResolutionList = GetResolutionInfoList();

	for (int32 Index=0; Index<ResolutionList.Num(); ++Index)
	{
		if (ResolutionList[Index].Resolution == CurrentResolution)
		{
			AO_LOG(LogJM, Log, TEXT("Index Found"));
			return Index;
		}
	}

	AO_LOG(LogJM, Warning, TEXT("Index Not Found (return -1)"));
	return -1;
}

FIntPoint UAO_GameSettingsManager::GetAppliedScreenResolution() const
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	if (const UAO_GameUserSettings* Settings = GetGameUserSettings())
	{
		return Settings->GetScreenResolution();
	}
	AO_LOG(LogJM, Log, TEXT("End"));
	return FIntPoint(1920, 1080);	// 포인터가 유효하지 않으면 1920x1080 해상도 반환
}

float UAO_GameSettingsManager::GetAudioVolume(const EAudioType AudioType) const
{
	AO_LOG(LogJM, Log, TEXT("Start"));
 	if (UAO_GameUserSettings* Settings = GetGameUserSettings())
 	{
 		switch (AudioType)
 		{
 		case EAudioType::Master:
 			return Settings->MasterVolume;
 		case EAudioType::Music:
 			return Settings->MusicVolume;
 		case EAudioType::SFX:
 			return Settings->SFXVolume;
 		case EAudioType::UI:
 			return Settings->UIVolume;
 		case EAudioType::Voice:
 			return Settings->VoiceVolume;
 		default:
 			AO_LOG(LogJM, Warning, TEXT("Invalid AudioType"));
 			return 1.0;
 		}
 	}
	AO_LOG(LogJM, Warning, TEXT("Failed to get game user settings."));
 	return 1.0;
}

void UAO_GameSettingsManager::SetAudioVolume(const EAudioType AudioType, const float NewVolume)
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	if (UAO_GameUserSettings* Settings = GetGameUserSettings())
	{
		const float ClampedVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
		switch (AudioType)
		{
		case EAudioType::Master:
			Settings->MasterVolume = ClampedVolume;
			break;
		case EAudioType::Music:
			Settings->MusicVolume = ClampedVolume;
			break;
		case EAudioType::SFX:
			Settings->SFXVolume = ClampedVolume;
			break;
		case EAudioType::UI:
			Settings->UIVolume = ClampedVolume;
			break;
		case EAudioType::Voice:
			Settings->VoiceVolume = ClampedVolume;
			break;
		default:
			AO_LOG(LogJM, Warning, TEXT("Invalid AudioType"));
			break;
		}
	}
	else
	{
		AO_LOG(LogJM, Warning, TEXT("Failed to get game user settings."));
	}
	AO_LOG(LogJM, Log, TEXT("End"));
}

const TArray<FResolutionInfo>& UAO_GameSettingsManager::GetResolutionInfoList()
{
	static TArray<FResolutionInfo> ResolutionInfoList;

	if (ResolutionInfoList.Num() == 0)
	{
		auto AddResolutionInfo = [&](const int32 Width, const int32 Height, const FString& Type)
		{
			FResolutionInfo Info;
			Info.Resolution = FIntPoint(Width, Height);
			Info.DisplayName = FString::Printf(TEXT("%d x %d (%s)"), Width, Height, *Type);
			ResolutionInfoList.Add(Info);
		};
		
		// 16:9 비율
		AddResolutionInfo(3840, 2160, TEXT("UHD"));
		AddResolutionInfo(2560, 1440, TEXT("QHD"));
		AddResolutionInfo(1920, 1080, TEXT("FHD"));
		AddResolutionInfo(1280, 720, TEXT("HD"));
		
		// 21:9 비율
		AddResolutionInfo(3440, 1440, TEXT("UW-QHD"));
		AddResolutionInfo(2560, 1080, TEXT("UW-FHD"));
		
		// 32:9 비율
		AddResolutionInfo(5120, 1440, TEXT("SUW-QHD"));
		AddResolutionInfo(3840, 1080, TEXT("SUW-FHD"));
		
		// 기타 비율
		AddResolutionInfo(1280, 1024, TEXT("5:4"));
		AddResolutionInfo(1024, 768, TEXT("4:3"));
		AddResolutionInfo(1366, 768, TEXT("Laptop Hi-DPI"));
	}
	return ResolutionInfoList;
}
