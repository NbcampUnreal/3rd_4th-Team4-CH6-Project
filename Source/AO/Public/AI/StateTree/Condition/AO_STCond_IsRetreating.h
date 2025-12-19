// AO_STCond_IsRetreating.h

#pragma once

#include "CoreMinimal.h"
#include "Conditions/StateTreeAIConditionBase.h"
#include "AO_STCond_IsRetreating.generated.h"

class AAO_Stalker;

/**
 * Stalker가 후퇴 모드인지 확인하는 Condition 인스턴스 데이터
 */
USTRUCT()
struct FAO_STCond_IsRetreating_InstanceData
{
	GENERATED_BODY()

	// 반전 여부
	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bInvert = false;
};

/**
 * Stalker가 후퇴 모드인지 확인하는 Condition
 * - 공격 후 후퇴 상태인지 체크
 */
USTRUCT(meta = (DisplayName = "AO Is Retreating", Category = "AI|AO"))
struct AO_API FAO_STCond_IsRetreating : public FStateTreeAIConditionBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAO_STCond_IsRetreating_InstanceData;

	FAO_STCond_IsRetreating() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

protected:
	AAO_Stalker* GetStalker(FStateTreeExecutionContext& Context) const;
};

