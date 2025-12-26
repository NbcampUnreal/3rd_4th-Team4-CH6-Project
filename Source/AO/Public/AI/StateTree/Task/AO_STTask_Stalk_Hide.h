// AO_STTask_Stalk_Hide.h

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

