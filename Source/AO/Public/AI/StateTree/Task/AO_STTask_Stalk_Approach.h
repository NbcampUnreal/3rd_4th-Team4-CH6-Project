//KSJ : AO_STTask_Stalk_Approach

#pragma once

#include "CoreMinimal.h"
#include "Tasks/StateTreeAITask.h"
#include "AO_STTask_Stalk_Approach.generated.h"

class AAO_StalkerController;

USTRUCT()
struct FAO_STTask_Stalk_Approach_InstanceData
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<AAO_StalkerController> Controller;

	UPROPERTY()
	bool bIsMoving = false;

	// 시선 체크 간격 타이머
	UPROPERTY()
	float LookCheckTimer = 0.f;

	// 시선 체크 간격 (초)
	UPROPERTY(EditAnywhere, Category = "Approach")
	float LookCheckInterval = 0.2f;

	// 공격 거리 (이 거리 이내로 들어오면 성공)
	UPROPERTY(EditAnywhere, Category = "Approach")
	float AttackRange = 200.f;

	// 시선 허용 각도 (도)
	UPROPERTY(EditAnywhere, Category = "Approach")
	float LookToleranceDegrees = 45.f;
};

/**
 * Stalker 접근 Task
 * - 플레이어가 안 보고 있을 때 접근
 * - 플레이어가 보면 Failed (Hide로 전환)
 * - 공격 거리 도달 시 Succeeded (Attack으로 전환)
 */
USTRUCT(meta = (DisplayName = "AO Stalker Approach", Category = "AI|AO|Stalker"))
struct AO_API FAO_STTask_Stalk_Approach : public FStateTreeAIActionTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAO_STTask_Stalk_Approach_InstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};

