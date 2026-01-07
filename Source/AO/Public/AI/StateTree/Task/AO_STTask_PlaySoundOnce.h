//KSJ : AO_STTask_PlaySoundOnce - StateTree에서 1회성 사운드 재생 Task (공용)

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "AO_STTask_PlaySoundOnce.generated.h"

class USoundBase;
class USoundAttenuation;

USTRUCT()
struct FAO_STTask_PlaySoundOnce_InstanceData
{
	GENERATED_BODY()

	// 재생할 사운드 (MetaSound Source 포함)
	UPROPERTY(EditAnywhere, Category = "Sound")
	TObjectPtr<USoundBase> Sound = nullptr;

	// 거리 감쇠/공간화 설정
	UPROPERTY(EditAnywhere, Category = "Sound")
	TObjectPtr<USoundAttenuation> AttenuationSettings = nullptr;

	// 볼륨 배수
	UPROPERTY(EditAnywhere, Category = "Sound", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float VolumeMultiplier = 1.0f;

	// 피치 배수
	UPROPERTY(EditAnywhere, Category = "Sound", meta = (ClampMin = "0.1", ClampMax = "3.0"))
	float PitchMultiplier = 1.0f;

	// Actor에 부착할지 여부 (권장: true)
	UPROPERTY(EditAnywhere, Category = "Sound")
	bool bAttachToOwner = true;

	// AttachPointName (소켓 이름). bAttachToOwner가 true일 때만 사용.
	UPROPERTY(EditAnywhere, Category = "Sound")
	FName AttachPointName = NAME_None;
};

/**
 * 공용 사운드 1회 재생 Task
 * - State Enter에서 “1회만” 재생하고 즉시 Succeeded 반환
 * - Chase 시작 시 Growl 1회, 발견/경고 사운드 1회 등에 사용
 */
USTRUCT(meta = (DisplayName = "AO: Play Sound Once", Category = "AO|Audio"))
struct AO_API FAO_STTask_PlaySoundOnce : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAO_STTask_PlaySoundOnce_InstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

private:
	// Owner가 Pawn/Controller/Actor 어느 쪽으로 들어오든, 실제 사운드 부착 대상 Actor를 반환.
	static AActor* ResolveOwnerActor(FStateTreeExecutionContext& Context);
};







