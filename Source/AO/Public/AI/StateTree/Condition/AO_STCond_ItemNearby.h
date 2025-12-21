// AO_STCond_ItemNearby.h

#pragma once

#include "CoreMinimal.h"
#include "Conditions/StateTreeAIConditionBase.h"
#include "AO_STCond_ItemNearby.generated.h"

class AAO_Crab;

/**
 * 주변에 줍을 수 있는 아이템이 있는지 확인하는 Condition 인스턴스 데이터
 */
USTRUCT()
struct FAO_STCond_ItemNearby_InstanceData
{
	GENERATED_BODY()

	// 아이템 탐색 반경
	UPROPERTY(EditAnywhere, Category = "Condition")
	float SearchRadius = 1500.f;

	// 반전 여부
	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bInvert = false;
};

/**
 * 주변에 줍을 수 있는 아이템이 있는지 확인하는 Condition
 */
USTRUCT(meta = (DisplayName = "AO Item Nearby", Category = "AI|AO"))
struct AO_API FAO_STCond_ItemNearby : public FStateTreeAIConditionBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAO_STCond_ItemNearby_InstanceData;

	FAO_STCond_ItemNearby() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

protected:
	AAO_Crab* GetCrab(FStateTreeExecutionContext& Context) const;
};
