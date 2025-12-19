// AO_STTask_Stalk_Ceiling.cpp

#include "AI/StateTree/Task/AO_STTask_Stalk_Ceiling.h"
#include "AI/Character/AO_Stalker.h"
#include "StateTreeExecutionContext.h"
#include "AIController.h"

EStateTreeRunStatus FAO_STTask_Stalk_Ceiling::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	
	AActor* Owner = Cast<AActor>(Context.GetOwner());
	if (APawn* Pawn = Cast<APawn>(Owner))
	{
		InstanceData.Stalker = Cast<AAO_Stalker>(Pawn);
	}
	else if (AController* Ctrl = Cast<AController>(Owner))
	{
		InstanceData.Stalker = Cast<AAO_Stalker>(Ctrl->GetPawn());
	}

	if (InstanceData.Stalker)
	{
		InstanceData.Stalker->SetCeilingMode(InstanceData.bEnableCeiling);
		return EStateTreeRunStatus::Running; // 상태 유지
	}

	return EStateTreeRunStatus::Failed;
}

void FAO_STTask_Stalk_Ceiling::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	if (InstanceData.Stalker)
	{
		// 상태를 나갈 때 천장 모드를 끄거나 유지할지 결정. 
		// 보통 다른 행동(공격 등)을 위해 내려와야 할 수 있으므로 끄는게 안전
		InstanceData.Stalker->SetCeilingMode(false);
	}
}

