// AO_STCond_InAttackRange.h

#pragma once

#include "CoreMinimal.h"
#include "Conditions/StateTreeAIConditionBase.h"
#include "AO_STCond_InAttackRange.generated.h"

class AAO_AggressiveAICtrl;

/**
 * 공격 범위 내 플레이어 Condition 인스턴스 데이터
 */
USTRUCT()
struct FAO_STCond_InAttackRange_InstanceData
{
	GENERATED_BODY()

	// 반전 여부 (공격 범위 밖일 때 true 반환)
	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bInvert = false;
};

/**
 * 공격 범위 내 플레이어 Condition
 * - 현재 추격 대상이 공격 범위 내에 있는지 확인
 */
USTRUCT(meta = (DisplayName = "AO Target In Attack Range", Category = "AI|AO"))
struct AO_API FAO_STCond_InAttackRange : public FStateTreeAIConditionBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAO_STCond_InAttackRange_InstanceData;

	FAO_STCond_InAttackRange() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

protected:
	AAO_AggressiveAICtrl* GetAggressiveController(FStateTreeExecutionContext& Context) const;
};

