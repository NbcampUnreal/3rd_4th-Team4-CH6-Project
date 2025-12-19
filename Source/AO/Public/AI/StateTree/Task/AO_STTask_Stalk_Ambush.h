// AO_STTask_Stalk_Ambush.h

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

	float AmbushTimer = 0.f;
};

/**
 * Stalker 기습 대기 Task
 * - 엄폐 상태에서 플레이어가 가까워지거나 등을 보일 때까지 대기
 * - 조건 만족 시 기습 공격 (GAS 실행)
 */
USTRUCT(meta = (DisplayName = "AO Stalker Ambush", Category = "AI|AO|Stalker"))
struct AO_API FAO_STTask_Stalk_Ambush : public FStateTreeAIActionTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAO_STTask_Stalk_Ambush_InstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};

