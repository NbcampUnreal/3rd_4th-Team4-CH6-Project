// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/StateTree/Condition/AO_STCond_HowlReceived.h"
#include "AI/Character/AO_Werewolf.h"
#include "AI/Component/AO_PackCoordComp.h"
#include "AIController.h"
#include "StateTreeExecutionContext.h"

bool FAO_STCond_HowlReceived::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	if (InstanceData.PackComp)
	{
		return InstanceData.PackComp->IsSurrounding();
	}

	return false;
}

void FAO_STCond_HowlReceived::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	
	// FStateTreeAIConditionBase는 AIController나 Pawn에 대한 접근 헬퍼를 제공하지 않으므로 직접 가져옵니다.
	// 하지만 AI Action Task와 달리 Condition은 보통 Context Actor(Owner)를 사용합니다.
	
	AActor* OwnerActor = Cast<AActor>(Context.GetOwner());
	AAO_Werewolf* Wolf = nullptr;

	if (APawn* Pawn = Cast<APawn>(OwnerActor))
	{
		Wolf = Cast<AAO_Werewolf>(Pawn);
	}
	else if (AController* Controller = Cast<AController>(OwnerActor))
	{
		Wolf = Cast<AAO_Werewolf>(Controller->GetPawn());
	}

	if (Wolf)
	{
		InstanceData.PackComp = Wolf->GetPackCoordComp();
	}
}
