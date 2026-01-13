//KSJ : AO_STTask_Stalk_Ceiling

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
		// 천장 모드 토글은 "1회성" 액션이므로 즉시 성공 처리해야
		// 상위 Selector가 Roam/Chase 같은 이동 태스크를 정상적으로 실행할 수 있다.
		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Failed;
}

void FAO_STTask_Stalk_Ceiling::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	if (InstanceData.Stalker)
	{
		// ExitState에서 무조건 끄면 Enter/Exit가 토글 경쟁을 일으켜
		// 천장 모드가 깜빡이거나 의도치 않게 해제된다.
		// 내려오는 동작은 bEnableCeiling=false로 별도 상태에서 명시적으로 수행한다.
	}
}

