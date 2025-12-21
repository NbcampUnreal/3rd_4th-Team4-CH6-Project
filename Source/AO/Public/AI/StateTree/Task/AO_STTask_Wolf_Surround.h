// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Tasks/StateTreeAITask.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "AO_STTask_Wolf_Surround.generated.h"

class UEnvQuery;
class UAO_PackCoordComp;
class AAO_AggressiveAICtrl;

USTRUCT(BlueprintType)
struct FAO_STTask_Wolf_Surround_InstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Settings")
	TObjectPtr<UEnvQuery> SurroundQuery = nullptr;

	UPROPERTY(EditAnywhere, Category = "Settings")
	float QueryInterval = 1.0f;

	UPROPERTY()
	TObjectPtr<UAO_PackCoordComp> PackComp = nullptr;

	UPROPERTY()
	TObjectPtr<AAO_AggressiveAICtrl> Controller = nullptr;

	float QueryTimer = 0.f;
	
	// EQS Request ID
	int32 QueryRequestID = INDEX_NONE;
};

/**
 * Werewolf Surround Task
 * - EQS를 사용해 포위/도주로 차단 지점으로 이동
 * - PackCoordComp의 상태가 'Attacking'이 되면 종료
 */
USTRUCT(meta = (DisplayName = "Werewolf: Surround & Block", Category = "AO|Werewolf"))
struct AO_API FAO_STTask_Wolf_Surround : public FStateTreeAIActionTaskBase
{
	GENERATED_BODY()

	typedef FAO_STTask_Wolf_Surround_InstanceData FInstanceDataType;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
