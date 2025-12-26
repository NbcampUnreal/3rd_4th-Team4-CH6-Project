// AO_STCond_PlayerLooking.h

#pragma once

#include "CoreMinimal.h"
#include "Conditions/StateTreeAIConditionBase.h"
#include "AO_STCond_PlayerLooking.generated.h"

class AAO_AggressiveAICtrl;

USTRUCT()
struct FAO_STCond_PlayerLooking_InstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bInvert = false;

	// 시야각 (Cosine 값)
	UPROPERTY(EditAnywhere, Category = "Condition")
	float FOVCosine = 0.5f; // 약 60도
};

/**
 * 플레이어가 AI를 보고 있는지 확인
 */
USTRUCT(meta = (DisplayName = "AO Player Looking At Me", Category = "AI|AO"))
struct AO_API FAO_STCond_PlayerLooking : public FStateTreeAIConditionBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAO_STCond_PlayerLooking_InstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};

