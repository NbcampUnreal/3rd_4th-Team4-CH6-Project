// AO_STCond_HasItem.h

#pragma once

#include "CoreMinimal.h"
#include "Conditions/StateTreeAIConditionBase.h"
#include "AO_STCond_HasItem.generated.h"

class AAO_Crab;

/**
 * Crab이 아이템을 들고 있는지 확인하는 Condition 인스턴스 데이터
 */
USTRUCT()
struct FAO_STCond_HasItem_InstanceData
{
	GENERATED_BODY()

	// 반전 여부 (true면 아이템이 없을 때 true 반환)
	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bInvert = false;
};

/**
 * Crab이 아이템을 들고 있는지 확인하는 Condition
 */
USTRUCT(meta = (DisplayName = "AO Has Item", Category = "AI|AO"))
struct AO_API FAO_STCond_HasItem : public FStateTreeAIConditionBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAO_STCond_HasItem_InstanceData;

	FAO_STCond_HasItem() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

protected:
	AAO_Crab* GetCrab(FStateTreeExecutionContext& Context) const;
};
