// AO_STCond_IsKidnapping.h

#pragma once

#include "CoreMinimal.h"
#include "Conditions/StateTreeAIConditionBase.h"
#include "AO_STCond_IsKidnapping.generated.h"

class AAO_Insect;

USTRUCT()
struct FAO_STCond_IsKidnapping_InstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bInvert = false;
};

USTRUCT(meta = (DisplayName = "AO Is Kidnapping", Category = "AI|AO"))
struct AO_API FAO_STCond_IsKidnapping : public FStateTreeAIConditionBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAO_STCond_IsKidnapping_InstanceData;

	FAO_STCond_IsKidnapping() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};

