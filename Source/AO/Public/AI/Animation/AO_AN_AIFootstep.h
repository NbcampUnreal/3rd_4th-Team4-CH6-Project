//KSJ : AO_AN_AIFootstep - AI 캐릭터용 범용 사운드 AnimNotify

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AO_AN_AIFootstep.generated.h"

class USoundAttenuation;

/**
 * AI 캐릭터용 범용 사운드 AnimNotify
 * 
 * 발걸음, 공격, 으르렁거림 등 다양한 사운드에 사용 가능
 * 
 * 주요 기능:
 * - 3D 공간 사운드 (Attenuation 지원)
 * - 볼륨/피치 조절
 * - 최대 재생 시간 제한 (긴 사운드 짧게 끊기)
 * 
 * 사용법:
 * - 애니메이션 타임라인에서 원하는 프레임에 추가
 * - Sound에 사운드 에셋 할당
 * - 필요시 Attenuation, Volume, Pitch, MaxDuration 조절
 */
UCLASS()
class AO_API UAO_AN_AIFootstep : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAO_AN_AIFootstep();

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;

protected:
	// 재생할 사운드 (SoundCue, MetaSound, WAV 등)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AO|Sound")
	TObjectPtr<USoundBase> Sound = nullptr;

	// 사운드 감쇠 설정 (3D 공간화, 거리 감쇠, 리버브 등)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AO|Sound")
	TObjectPtr<USoundAttenuation> AttenuationSettings = nullptr;

	// 볼륨 배수 (기본 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AO|Sound", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float VolumeMultiplier = 1.0f;

	// 피치 배수 (낮을수록 무거운 느낌, 기본 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AO|Sound", meta = (ClampMin = "0.1", ClampMax = "3.0"))
	float PitchMultiplier = 1.0f;

	// 최대 재생 시간 (초). 0이면 제한 없음 (사운드 끝까지 재생)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AO|Sound", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float MaxDuration = 0.0f;
};
