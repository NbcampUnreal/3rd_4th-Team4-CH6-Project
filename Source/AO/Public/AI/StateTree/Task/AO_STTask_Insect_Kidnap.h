// AO_STTask_Insect_Kidnap.h

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "GameplayTagContainer.h"
#include "AO_STTask_Insect_Kidnap.generated.h"

class UAbilitySystemComponent;

USTRUCT()
struct FAO_STTask_Insect_Kidnap_InstanceData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AActor> TargetActor = nullptr;
};

/**
 * Insect 납치 실행 Task
 * - GAS Ability 트리거
 */
USTRUCT(meta = (DisplayName = "AO Insect Kidnap Task", Category = "AI|AO"))
struct AO_API FAO_STTask_Insect_Kidnap : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAO_STTask_Insect_Kidnap_InstanceData;

	FAO_STTask_Insect_Kidnap() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Setting")
	FGameplayTag KidnapAbilityTag;
};

