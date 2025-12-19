// AO_STEval_InsectContext.h

#pragma once

#include "CoreMinimal.h"
#include "StateTreeEvaluatorBase.h"
#include "AO_STEval_InsectContext.generated.h"

class AAO_Insect;

USTRUCT()
struct FAO_STEval_InsectContext_InstanceData
{
	GENERATED_BODY()

	// 현재 납치 중인지
	UPROPERTY(EditAnywhere, Category = "Output")
	bool bIsKidnapping = false;

	// 계산된 안전 위치 (납치 후 이동할 곳)
	UPROPERTY(EditAnywhere, Category = "Output")
	FVector SafeLocation = FVector::ZeroVector;
};

USTRUCT(meta = (DisplayName = "AO Insect Context Evaluator"))
struct AO_API FAO_STEval_InsectContext : public FStateTreeEvaluatorCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAO_STEval_InsectContext_InstanceData;

	FAO_STEval_InsectContext() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};

