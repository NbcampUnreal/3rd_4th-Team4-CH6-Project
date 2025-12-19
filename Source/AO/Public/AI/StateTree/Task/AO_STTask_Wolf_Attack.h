// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Tasks/StateTreeAITask.h"
#include "AO_STTask_Wolf_Attack.generated.h"

class AAO_AggressiveAICtrl;

USTRUCT(BlueprintType)
struct FAO_STTask_Wolf_Attack_InstanceData
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<AAO_AggressiveAICtrl> Controller = nullptr;
};

/**
 * Werewolf Attack Task
 * - 추격 및 근접 공격
 */
USTRUCT(meta = (DisplayName = "Werewolf: Attack", Category = "AO|Werewolf"))
struct AO_API FAO_STTask_Wolf_Attack : public FStateTreeAIActionTaskBase
{
	GENERATED_BODY()

	typedef FAO_STTask_Wolf_Attack_InstanceData FInstanceDataType;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};
