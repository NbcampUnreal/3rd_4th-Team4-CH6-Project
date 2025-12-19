// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Tasks/StateTreeAITask.h"
#include "AO_STTask_Wolf_Howl.generated.h"

class UAO_PackCoordComp;
class AAO_AggressiveAICtrl;

USTRUCT(BlueprintType)
struct FAO_STTask_Wolf_Howl_InstanceData
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UAO_PackCoordComp> PackComp = nullptr;

	UPROPERTY()
	TObjectPtr<AAO_AggressiveAICtrl> Controller = nullptr;

	UPROPERTY(EditAnywhere, Category = "Settings")
	float HowlDuration = 2.0f; // 애니메이션 길이 등

	float Timer = 0.f;
};

/**
 * Werewolf Howl Task
 * - 제자리에서 Howl 실행 및 전파
 */
USTRUCT(meta = (DisplayName = "Werewolf: Howl", Category = "AO|Werewolf"))
struct AO_API FAO_STTask_Wolf_Howl : public FStateTreeAIActionTaskBase
{
	GENERATED_BODY()

	typedef FAO_STTask_Wolf_Howl_InstanceData FInstanceDataType;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};
