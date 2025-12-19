// AO_STCond_InCeiling.h

#pragma once

#include "CoreMinimal.h"
#include "Conditions/StateTreeAIConditionBase.h"
#include "AO_STCond_InCeiling.generated.h"

class UAO_CeilingMoveComponent;

USTRUCT()
struct FAO_STCond_InCeiling_InstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bInvert = false;
};

/**
 * Stalker가 현재 천장 모드인지 확인
 */
USTRUCT(meta = (DisplayName = "AO In Ceiling Mode", Category = "AI|AO"))
struct AO_API FAO_STCond_InCeiling : public FStateTreeAIConditionBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAO_STCond_InCeiling_InstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};

