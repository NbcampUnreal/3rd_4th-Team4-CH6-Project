//KSJ : AO_STTask_PlaySoundInterval - StateTree에서 간헐 반복 사운드 재생 Task (공용)

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "AO_STTask_PlaySoundInterval.generated.h"

class UAudioComponent;
class USoundAttenuation;
class USoundBase;

USTRUCT()
struct FAO_STTask_PlaySoundInterval_InstanceData
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

	// EnterState에서 즉시 1회 재생할지 여부
	UPROPERTY(EditAnywhere, Category = "Interval")
	bool bPlayOnEnter = false;

	// 반복 간격 범위 (초)
	UPROPERTY(EditAnywhere, Category = "Interval", meta = (ClampMin = "0.1", ClampMax = "60.0"))
	float MinInterval = 2.0f;

	UPROPERTY(EditAnywhere, Category = "Interval", meta = (ClampMin = "0.1", ClampMax = "60.0"))
	float MaxInterval = 6.0f;

	// 겹침 방지: 재생 중이면 다음 트리거를 스킵
	UPROPERTY(EditAnywhere, Category = "Interval")
	bool bPreventOverlap = true;

	// 상태 종료 시(ExitState) 재생 중인 컴포넌트를 정지할지
	UPROPERTY(EditAnywhere, Category = "Interval")
	bool bStopOnExit = true;

	// 내부 타이머
	UPROPERTY()
	float Timer = 0.f;

	// 겹침 방지를 위한 재생용 컴포넌트 (Task 실행 중만 유지)
	UPROPERTY()
	TObjectPtr<UAudioComponent> AudioComponent = nullptr;
};

/**
 * 공용 간헐 반복 사운드 Task
 * - Chase 상태에서 2~6초 랜덤 간격으로 Growl 재생 등
 * - bPreventOverlap=true면 현재 재생 중인 경우 중복 재생을 피함
 */
USTRUCT(meta = (DisplayName = "AO: Play Sound Interval", Category = "AO|Audio"))
struct AO_API FAO_STTask_PlaySoundInterval : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAO_STTask_PlaySoundInterval_InstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

private:
	static AActor* ResolveOwnerActor(FStateTreeExecutionContext& Context);
	static float ComputeNextInterval(const float MinInterval, const float MaxInterval);
	static void StopAndDestroyAudioComponent(TObjectPtr<UAudioComponent>& InOutAudioComponent);
};







