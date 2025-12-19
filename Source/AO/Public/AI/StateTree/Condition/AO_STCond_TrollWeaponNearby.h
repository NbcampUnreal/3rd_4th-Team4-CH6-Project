#pragma once

#include "CoreMinimal.h"
#include "Conditions/StateTreeAIConditionBase.h"
#include "AO_STCond_TrollWeaponNearby.generated.h"

class AAO_Troll;

/**
 * Troll 주변에 주울 수 있는 무기가 있는지 확인하는 Condition
 */
USTRUCT()
struct FAO_STCond_TrollWeaponNearby_InstanceData
{
	GENERATED_BODY()

	// 무기 탐색 반경 (0이면 시야만 확인, 값이 있으면 거리 검색도 수행)
	UPROPERTY(EditAnywhere, Category = "Condition")
	float SearchRadius = 2000.f;

	// 반전 여부
	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bInvert = false;
};

USTRUCT(meta = (DisplayName = "AO Troll Weapon Nearby", Category = "AI|AO|Troll"))
struct AO_API FAO_STCond_TrollWeaponNearby : public FStateTreeAIConditionBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAO_STCond_TrollWeaponNearby_InstanceData;

	FAO_STCond_TrollWeaponNearby() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

protected:
	AAO_Troll* GetTroll(FStateTreeExecutionContext& Context) const;
};

