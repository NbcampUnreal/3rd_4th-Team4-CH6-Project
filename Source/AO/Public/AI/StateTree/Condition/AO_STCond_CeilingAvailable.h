// AO_STCond_CeilingAvailable.h

#pragma once

#include "CoreMinimal.h"
#include "Conditions/StateTreeAIConditionBase.h"
#include "AO_STCond_CeilingAvailable.generated.h"

class UAO_CeilingMoveComponent;

USTRUCT()
struct FAO_STCond_CeilingAvailable_InstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bInvert = false;
};

/**
 * Stalker가 현재 위치에서 천장으로 이동 가능한지 확인
 * (천장이 있는지 체크)
 */
USTRUCT(meta = (DisplayName = "AO Ceiling Available", Category = "AI|AO"))
struct AO_API FAO_STCond_CeilingAvailable : public FStateTreeAIConditionBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAO_STCond_CeilingAvailable_InstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};

