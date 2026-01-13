// AO_GameUserSettings.h (장주만)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "AO_GameUserSettings.generated.h"

// 프로젝트의 품질 수준을 정의하는 Enum
UENUM(BlueprintType)
enum class EScalabilityLevel : uint8
{
	Low			UMETA(DisplayName = "Low"),
	Medium		UMETA(DisplayName = "Medium"),
	High		UMETA(DisplayName = "High"),
	Epic		UMETA(DisplayName = "Epic"),
	Cinematic	UMETA(DisplayName = "Cinematic"),
};

/* UPROPERTY(Config)를 사용하여 설정 파일에 자동으로 저장 및 로드됨
 * 이 클래스의 인스턴스를 저장하고 로드할 때 사용할 .ini 파일의 이름을 지정함 (GameUserSettings.ini) */
UCLASS(config=GameUserSettings)
class AO_API UAO_GameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()

public:
	static TObjectPtr<UAO_GameUserSettings> GetGameUserSettings();	// UFUNCTION 으로 쓸거면 다시 raw pointer로 전환

public:
	virtual void ApplyCustomSettings();		// NOTE: 엔진 정의 함수 아니고, 사용자 정의 함수임
	
public:
	/* 오디오 볼륨, 마우스 감도 등은 UGameUserSettings에서 기본적으로 제공하지 않기 때문에 확장함 */
	UPROPERTY(Config, BlueprintReadWrite, Category = "AO|Audio")
	float MasterVolume = 1.0f;

	UPROPERTY(Config, BlueprintReadWrite, Category = "AO|Audio")
	float MusicVolume = 1.0f;

	UPROPERTY(Config, BlueprintReadWrite, Category = "AO|Audio")
	float SFXVolume = 1.0f;

	UPROPERTY(Config, BlueprintReadWrite, Category = "AO|Audio")
	float UIVolume = 1.0f;

	UPROPERTY(Config, BlueprintReadWrite, Category = "AO|Audio")
	float VoiceVolume = 1.0f;

	UPROPERTY(Config, BlueprintReadWrite, Category = "AO|Audio")
	float AmbientVolume = 1.0f;

	UPROPERTY(Config, BlueprintReadWrite, Category = "AO|Upscaling")
	int32 UpscalingOption = 0; // Built-in(Default)

	UPROPERTY(Config, BlueprintReadWrite, Category = "AO|Upscaling|NVIDIA")
	int32 DlssOption = 2;	// Auto(Default)

	UPROPERTY(Config, BlueprintReadWrite, Category = "AO|Upscaling|NVIDIA")
	float DlssSharpness = 0.0f;
	
	UPROPERTY(Config, BlueprintReadWrite, Category = "AO|Upscaling|NVIDIA")
	bool bEnableRayReconstruction = false;
	
	UPROPERTY(Config, BlueprintReadWrite, Category = "AO|Upscaling|NVIDIA")
	int32 ImageScalingOption = 2;	// Ultra Quality(Default)

	UPROPERTY(Config, BlueprintReadWrite, Category = "AO|Upscaling|NVIDIA")
	float ImageScalingCustomResolution = 100.0f;
	
	UPROPERTY(Config, BlueprintReadWrite, Category = "AO|Upscaling|NVIDIA")
	float ImageScalingSharpness = 0.0f;

	UPROPERTY(Config, BlueprintReadWrite, Category = "AO|Upscaling|FSR")
	bool bEnableFsrFrameGeneration = false;

	UPROPERTY(Config, BlueprintReadWrite, Category = "AO|Upscaling|FSR")
	int32 FsrOption = 1;

	UPROPERTY(Config, BlueprintReadWrite, Category = "AO|Upscaling|BuiltIn")
	int32 BuiltInAntiAliasingOption = 0;

	UPROPERTY(Config, BlueprintReadWrite, Category = "AO|Upscaling|BuiltIn")
	float BuiltInResolutionScale = 100.0f;

	UPROPERTY(Config, BlueprintReadWrite, Category = "AO|Voice")
	bool bIsEnableVoiceChat = false;

	UPROPERTY(Config, BlueprintReadWrite, Category = "AO|Language")
	int32 Language = 0;
	
};
