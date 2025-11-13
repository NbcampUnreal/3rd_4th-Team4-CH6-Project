// AO_GameSettingsManager.h (장주만)

#pragma once

#include "CoreMinimal.h"
#include "AO_GameUserSettings.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AO_GameSettingsManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSettingsApplied);

/**
 * 게임의 모든 설정을 관리하는 전역 매니저
 * 설정값 변경, 적용, 시스템 통합 로직을 담당함
 */
UCLASS()
class AO_API UAO_GameSettingsManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	UFUNCTION(BlueprintPure, Category = "Game Settings")	
	UAO_GameUserSettings* GetGameUserSettings() const;		// StalePointer를 방지하기 위해 캐싱하는 것보다 매번 Get 하는게 낫다(성능차이 없음) 

	UFUNCTION(BlueprintCallable, Category = "Game Settings")
	void ApplyAndSaveAllSettings();

	UFUNCTION(BlueprintCallable, Category = "Game Settings")
	void RevertAndApplyDefaultSettings();

	UFUNCTION(BlueprintCallable, Category = "Game Settings|Graphics")
	void SetOverallScalability(EScalabilityLevel NewLevel);

	UFUNCTION(BlueprintCallable, Category = "Game Settings|Audio")
	void SetMasterVolume(float NewVolume);

	UFUNCTION(BlueprintCallable, Category = "Game Settings|Gameplay")
	void SetMouseSensitivity(float NewSensitivity);
	
public:
	UPROPERTY(BlueprintAssignable, Category = "Game Settings|Events")
	FOnSettingsApplied OnSettingsApplied;		// 설정이 성공적으로 적용되고 저장되었을 때 호출되는 델리게이트

	
};
