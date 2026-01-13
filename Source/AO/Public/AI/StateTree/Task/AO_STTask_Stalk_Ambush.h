//KSJ : AO_STTask_Stalk_Ambush

#pragma once

#include "CoreMinimal.h"
#include "Tasks/StateTreeAITask.h"
#include "AO_STTask_Stalk_Ambush.generated.h"

class AAO_StalkerController;

USTRUCT()
struct FAO_STTask_Stalk_Ambush_InstanceData
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<AAO_StalkerController> Controller;

	// 공격 중인지 여부 (Troll Attack Task 참고)
	UPROPERTY()
	bool bIsAttacking = false;

	UPROPERTY()
	bool bWaitingForAttackEnd = false;
};

/**
 * Stalker 공격 실행 Task (구 Ambush)
 * - EnterState에서 즉시 공격 실행 (GAS)
 * - 공격 완료 대기
 */
USTRUCT(meta = (DisplayName = "AO Stalker Attack (Ambush)", Category = "AI|AO|Stalker"))
struct AO_API FAO_STTask_Stalk_Ambush : public FStateTreeAIActionTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAO_STTask_Stalk_Ambush_InstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};
