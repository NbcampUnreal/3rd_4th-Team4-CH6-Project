// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Conditions/StateTreeAIConditionBase.h"
#include "AO_STCond_HowlReceived.generated.h"

class UAO_PackCoordComp;

USTRUCT(BlueprintType)
struct FAO_STCond_HowlReceived_InstanceData
{
	GENERATED_BODY()

	// AI Condition Base는 자동으로 AIController/Pawn Context를 처리하므로 Actor 명시적 바인딩이 필수적이진 않지만,
	// PackComp를 캐싱하기 위해 멤버 변수는 유지합니다.
	UPROPERTY()
	TObjectPtr<UAO_PackCoordComp> PackComp = nullptr;
};

/**
 * Werewolf가 Pack 모드(포위 상태)인지 확인하는 조건
 */
USTRUCT(meta = (DisplayName = "Werewolf: Is Pack Mode Active", Category = "AO|Werewolf"))
struct AO_API FAO_STCond_HowlReceived : public FStateTreeAIConditionBase
{
	GENERATED_BODY()

	typedef FAO_STCond_HowlReceived_InstanceData FInstanceDataType;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
	virtual void EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
