// AO_STCond_HasWeapon.h

#pragma once

#include "CoreMinimal.h"
#include "Conditions/StateTreeAIConditionBase.h"
#include "AO_STCond_HasWeapon.generated.h"

class AAO_TrollController;

/**
 * Troll 무기 소지 여부 Condition 인스턴스 데이터
 */
USTRUCT()
struct FAO_STCond_HasWeapon_InstanceData
{
	GENERATED_BODY()

	// 반전 여부 (무기가 없을 때 true 반환)
	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bInvert = false;
};

/**
 * Troll 무기 소지 여부 Condition
 * - Troll이 무기를 들고 있는지 확인
 */
USTRUCT(meta = (DisplayName = "AO Has Weapon", Category = "AI|AO|Troll"))
struct AO_API FAO_STCond_HasWeapon : public FStateTreeAIConditionBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAO_STCond_HasWeapon_InstanceData;

	FAO_STCond_HasWeapon() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

protected:
	AAO_TrollController* GetTrollController(FStateTreeExecutionContext& Context) const;
};

