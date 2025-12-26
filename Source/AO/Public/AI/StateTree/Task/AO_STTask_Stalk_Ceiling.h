// AO_STTask_Stalk_Ceiling.h

#pragma once

#include "CoreMinimal.h"
#include "Tasks/StateTreeAITask.h"
#include "AO_STTask_Stalk_Ceiling.generated.h"

class AAO_Stalker;

USTRUCT()
struct FAO_STTask_Stalk_Ceiling_InstanceData
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<AAO_Stalker> Stalker;

	UPROPERTY(EditAnywhere, Category = "Ceiling")
	bool bEnableCeiling = true;
};

/**
 * Stalker 천장 이동 모드 전환 Task
 */
USTRUCT(meta = (DisplayName = "AO Stalker Ceiling Mode", Category = "AI|AO|Stalker"))
struct AO_API FAO_STTask_Stalk_Ceiling : public FStateTreeAIActionTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAO_STTask_Stalk_Ceiling_InstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};

