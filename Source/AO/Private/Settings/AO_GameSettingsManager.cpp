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
		Settings->SaveSettings();									// 디스크에 설정 영구 저장
		OnSettingsApplied.Broadcast();								// 설적 적용 완료 이벤트를 발생시켜 알림
		AO_LOG(LogJM, Log, TEXT("All Settings Applied and Saved"));

		// TODO: 만약 해상도 변경처럼 재시작이 필요한 설정이 변경되었다면,
		// 사용자에게 재시작 알림을 띄우는 로직을 여기에 추가해야 함
		// TODO: 꼭 재시작을 해야하는가? 아니면 레벨 재 로드로 설정 적용이 가능한가?
	}
	else
	{
		AO_LOG(LogJM, Warning, TEXT("Failed to get game user settings."));
	}
	AO_LOG(LogJM, Log, TEXT("End"));
}

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
