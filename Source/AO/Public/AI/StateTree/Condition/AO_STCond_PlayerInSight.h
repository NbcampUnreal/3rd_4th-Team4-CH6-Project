// AO_STCond_PlayerInSight.h

#pragma once

#include "CoreMinimal.h"
#include "Conditions/StateTreeAIConditionBase.h"
#include "AO_STCond_PlayerInSight.generated.h"

class AAO_AIControllerBase;

/**
 * 시야 내 플레이어 Condition 인스턴스 데이터
 */
USTRUCT()
struct FAO_STCond_PlayerInSight_InstanceData
{
	GENERATED_BODY()

	// 반전 여부 (시야에 플레이어가 없을 때 true 반환)
	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bInvert = false;
};

/**
 * 시야 내 플레이어 Condition
 * - AI 시야 안에 플레이어가 있는지 확인
 */
USTRUCT(meta = (DisplayName = "AO Player In Sight", Category = "AI|AO"))
struct AO_API FAO_STCond_PlayerInSight : public FStateTreeAIConditionBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAO_STCond_PlayerInSight_InstanceData;

	FAO_STCond_PlayerInSight() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

protected:
	AAO_AIControllerBase* GetAIController(FStateTreeExecutionContext& Context) const;
};

