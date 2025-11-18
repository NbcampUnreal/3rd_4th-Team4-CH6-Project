// AO_GameSettingsManager.h (장주만)

#pragma once

#include "CoreMinimal.h"
#include "AO_GameUserSettings.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AO_GameSettingsManager.generated.h"


USTRUCT(BlueprintType)
struct FResolutionInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Resolution")
	FIntPoint Resolution = FIntPoint(0, 0); // 해상도 값 (1920x1080)

	UPROPERTY(BlueprintReadOnly, Category = "Resolution")
	FString DisplayName = TEXT(""); // 표시할 이름 (1920 x 1080 (FHD))
};


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

	UFUNCTION(BlueprintCallable, Category = "Game Settings|Apply")
	void ApplyAndSaveAllSettings();

	UFUNCTION(BlueprintCallable, Category = "Game Settings|Apply")
	void ApplyResolutionSettings();		// 무거움 (화면 새로고침)

	UFUNCTION(BlueprintCallable, Category = "Game Settings|Apply")
	void ApplyNonResolutionSettings();		// 가벼움

	/*UFUNCTION(BlueprintCallable, Category = "Game Settings")
	void RevertAndApplyDefaultSettings();*/

	// 기본값으로 초기화
	UFUNCTION(BlueprintCallable, Category = "Game Settings")
	void SetToDefaults();


	
	// Scalability (0 ~ 4, low ~ cine)
	UFUNCTION(BlueprintCallable, Category = "Game Settings|Graphics")
	void SetOverallScalability(EScalabilityLevel NewLevel);

	UFUNCTION(BlueprintCallable, Category = "Game Settings|Graphics")
	void SetAntiAliasingScalability(EScalabilityLevel NewLevel);

	UFUNCTION(BlueprintCallable, Category = "Game Settings|Graphics")
	void SetViewDistanceScalability(EScalabilityLevel NewLevel);

	UFUNCTION(BlueprintCallable, Category = "Game Settings|Graphics")
	void SetShadowScalability(EScalabilityLevel NewLevel);

	UFUNCTION(BlueprintCallable, Category = "Game Settings|Graphics")
	void SetGlobalIlluminationScalability(EScalabilityLevel NewLevel);

	UFUNCTION(BlueprintCallable, Category = "Game Settings|Graphics")
	void SetReflectionScalability(EScalabilityLevel NewLevel);

	UFUNCTION(BlueprintCallable, Category = "Game Settings|Graphics")
	void SetTextureScalability(EScalabilityLevel NewLevel);

	UFUNCTION(BlueprintCallable, Category = "Game Settings|Graphics")
	void SetVisualEffectScalability(EScalabilityLevel NewLevel);

	UFUNCTION(BlueprintCallable, Category = "Game Settings|Graphics")
	void SetPostProcessingScalability(EScalabilityLevel NewLevel);

	UFUNCTION(BlueprintCallable, Category = "Game Settings|Graphics")
	void SetFoliageScalability(EScalabilityLevel NewLevel);

	UFUNCTION(BlueprintCallable, Category = "Game Settings|Graphics")
	void SetShadingScalability(EScalabilityLevel NewLevel);

	// VSync
	UFUNCTION(BlueprintCallable, Category = "Game Settings|Graphics")
	void SetVSyncEnabled(bool bEnable);

	// Screen Mode
	UFUNCTION(BlueprintCallable, Category = "Game Settings|Gameplay")
	void SetFullscreenMode(EWindowMode::Type InFullscreenMode);

	// Screen Resolution
	UFUNCTION(BlueprintCallable, Category = "Game Settings|Gameplay")
	void SetScreenResolutionByIndex(int32 ResolutionIndex);

	UFUNCTION(BlueprintCallable, Category = "Game Settings|Gameplay")
	TArray<FResolutionInfo> GetSupportedScreenResolutionInfos() const;

	UFUNCTION(BlueprintCallable, Category = "Game Settings|Gameplay")
	int32 GetCurrentResolutionIndex() const;

	UFUNCTION(BlueprintCallable, Category = "Game Settings|Gameplay")
	FIntPoint GetAppliedScreenResolution() const;

	
	
	// 커스텀 추가기능
	UFUNCTION(BlueprintCallable, Category = "Game Settings|Audio")
	void SetMasterVolume(float NewVolume);

	UFUNCTION(BlueprintCallable, Category = "Game Settings|Gameplay")
	void SetMouseSensitivity(float NewSensitivity);
	
public:
	UPROPERTY(BlueprintAssignable, Category = "Game Settings|Events")
	FOnSettingsApplied OnSettingsApplied;		// 설정이 성공적으로 적용되고 저장되었을 때 호출되는 델리게이트

private:
	static const TArray<FResolutionInfo>& GetResolutionInfoList();
	
};
