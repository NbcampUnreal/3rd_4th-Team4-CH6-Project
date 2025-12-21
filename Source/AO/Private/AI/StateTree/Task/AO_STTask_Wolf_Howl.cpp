// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/StateTree/Task/AO_STTask_Wolf_Howl.h"
#include "AI/Character/AO_Werewolf.h"
#include "AI/Controller/AO_AggressiveAICtrl.h"
#include "AI/Component/AO_PackCoordComp.h"
#include "StateTreeExecutionContext.h"
#include "Character/AO_PlayerCharacter.h"
#include "AIController.h"

EStateTreeRunStatus FAO_STTask_Wolf_Howl::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	
	// FStateTreeAIActionTaskBase는 Context에서 Owner Actor를 가져오는 것을 전제함
	AActor* OwnerActor = Cast<AActor>(Context.GetOwner());
	AAO_Werewolf* Wolf = nullptr;
	AAO_AggressiveAICtrl* Ctrl = nullptr;

	if (APawn* Pawn = Cast<APawn>(OwnerActor))
	{
		Wolf = Cast<AAO_Werewolf>(Pawn);
		Ctrl = Cast<AAO_AggressiveAICtrl>(Pawn->GetController());
	}
	else if (AController* Controller = Cast<AController>(OwnerActor))
	{
		Ctrl = Cast<AAO_AggressiveAICtrl>(Controller);
		Wolf = Cast<AAO_Werewolf>(Controller->GetPawn());
	}

	InstanceData.Controller = Ctrl;

	if (Wolf)
	{
		InstanceData.PackComp = Wolf->GetPackCoordComp();
	}

	if (InstanceData.PackComp && Ctrl)
	{
		// Howl 전파
		InstanceData.PackComp->BroadcastHowl(Ctrl->GetChaseTarget());
		
		// TODO: Play Animation Montage (GAS Ability로 대체 예정)
		// UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(...)
	}
	else
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.Timer = 0.f;

	// 이동 정지
	if (Ctrl)
	{
		Ctrl->StopMovement();
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FAO_STTask_Wolf_Howl::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	InstanceData.Timer += DeltaTime;

	// 애니메이션 시간만큼 대기 후 성공 처리
	if (InstanceData.Timer >= InstanceData.HowlDuration)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Running;
}
