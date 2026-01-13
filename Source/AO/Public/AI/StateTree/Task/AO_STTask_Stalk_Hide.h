//KSJ : AO_STTask_Stalk_Hide

#pragma once

#include "CoreMinimal.h"
#include "Tasks/StateTreeAITask.h"
#include "AO_STTask_Stalk_Hide.generated.h"

class UEnvQuery;
class AAO_StalkerController;

USTRUCT()
struct FAO_STTask_Stalk_Hide_InstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "EQS")
	TObjectPtr<UEnvQuery> HideQuery;

	UPROPERTY()
	TObjectPtr<AAO_StalkerController> Controller;

	UPROPERTY()
	bool bIsMoving = false;

	// 플레이어 접근 체크 타이머
	UPROPERTY()
	float PlayerProximityCheckTimer = 0.f;

	// 현재 엄폐 위치
	UPROPERTY()
	FVector CurrentHideLocation = FVector::ZeroVector;

	// KSJ: EQS 요청 후 결과를 기다리는 중인지
	UPROPERTY()
	bool bAwaitingEQSResult = false;

	// EQS 대기 시간 (무한 대기 방지)
	UPROPERTY()
	float EQSWaitTimer = 0.f;

	UPROPERTY(EditAnywhere, Category = "EQS")
	float EQSWaitTimeout = 1.0f;

	// 나를 보고 있는 플레이어 (뒷걸음질 시 바라볼 대상)
	UPROPERTY()
	TWeakObjectPtr<AActor> LookingPlayer = nullptr;

	// 뒷걸음질 모드 활성화 여부
	UPROPERTY(EditAnywhere, Category = "Movement")
	bool bEnableBackpedal = true;

	// 뒷걸음질 시 회전 보간 속도 (도/초)
	UPROPERTY(EditAnywhere, Category = "Movement")
	float BackpedalRotationSpeed = 360.f;
};

/**
 * Stalker 은신/엄폐 Task
 * - EQS를 통해 플레이어 시야에서 숨을 수 있는 엄폐물로 이동
 */
USTRUCT(meta = (DisplayName = "AO Stalker Hide", Category = "AI|AO|Stalker"))
struct AO_API FAO_STTask_Stalk_Hide : public FStateTreeAIActionTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAO_STTask_Stalk_Hide_InstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};

