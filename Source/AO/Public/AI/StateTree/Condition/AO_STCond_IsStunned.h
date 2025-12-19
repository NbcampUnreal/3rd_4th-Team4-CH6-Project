// AO_STCond_IsStunned.h

#pragma once

#include "CoreMinimal.h"
#include "Conditions/StateTreeAIConditionBase.h"
#include "AO_STCond_IsStunned.generated.h"

class AAO_AICharacterBase;

/**
 * AI가 기절 상태인지 확인하는 Condition 인스턴스 데이터
 */
USTRUCT()
struct FAO_STCond_IsStunned_InstanceData
{
	GENERATED_BODY()

	// 반전 여부
	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bInvert = false;
};

/**
 * AI가 기절 상태인지 확인하는 Condition
 */
USTRUCT(meta = (DisplayName = "AO Is Stunned", Category = "AI|AO"))
struct AO_API FAO_STCond_IsStunned : public FStateTreeAIConditionBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAO_STCond_IsStunned_InstanceData;

	FAO_STCond_IsStunned() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

protected:
	AAO_AICharacterBase* GetAICharacter(FStateTreeExecutionContext& Context) const;
};
