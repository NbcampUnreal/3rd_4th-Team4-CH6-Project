// AO_STTask_AIMoveTo.cpp

#include "AI/StateTree/Task/AO_STTask_AIMoveTo.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "StateTreeExecutionContext.h"
#include "AO_Log.h"

EStateTreeRunStatus FAO_STTask_AIMoveTo::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FAO_STTask_AIMoveTo_InstanceData& InstanceData = Context.GetInstanceData<FAO_STTask_AIMoveTo_InstanceData>(*this);

	// AIController가 유효해야 이동 가능
	AAIController* AIController = GetAIController(Context);
	if (!ensureMsgf(AIController, TEXT("AIMoveTo: AIController is null")))
	{
		return EStateTreeRunStatus::Failed;
	}

	// 목표 위치가 유효해야 이동 가능
	if (InstanceData.TargetLocation.IsZero())
	{
		AO_LOG(LogKSJ, Warning, TEXT("AIMoveTo: TargetLocation is zero"));
		return EStateTreeRunStatus::Failed;
	}

	EPathFollowingRequestResult::Type Result = AIController->MoveToLocation(InstanceData.TargetLocation, InstanceData.AcceptanceRadius);

	if (Result == EPathFollowingRequestResult::Failed)
	{
		AO_LOG(LogKSJ, Warning, TEXT("AIMoveTo Failed to start movement"));
		return EStateTreeRunStatus::Failed;
	}

	if (Result == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	InstanceData.bIsMoving = true;
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FAO_STTask_AIMoveTo::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FAO_STTask_AIMoveTo_InstanceData& InstanceData = Context.GetInstanceData<FAO_STTask_AIMoveTo_InstanceData>(*this);

	AAIController* AIController = GetAIController(Context);
	if (!AIController)
	{
		return EStateTreeRunStatus::Failed;
	}

	UPathFollowingComponent* PathComp = AIController->GetPathFollowingComponent();
	if (!PathComp)
	{
		return EStateTreeRunStatus::Failed;
	}

	if (PathComp->GetStatus() == EPathFollowingStatus::Idle)
	{
		InstanceData.bIsMoving = false;
		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Running;
}

void FAO_STTask_AIMoveTo::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FAO_STTask_AIMoveTo_InstanceData& InstanceData = Context.GetInstanceData<FAO_STTask_AIMoveTo_InstanceData>(*this);

	if (InstanceData.bIsMoving)
	{
		if (AAIController* AIController = GetAIController(Context))
		{
			AIController->StopMovement();
		}
		InstanceData.bIsMoving = false;
	}
}

AAIController* FAO_STTask_AIMoveTo::GetAIController(FStateTreeExecutionContext& Context) const
{
	if (AActor* Owner = Cast<AActor>(Context.GetOwner()))
	{
		if (APawn* Pawn = Cast<APawn>(Owner))
		{
			return Cast<AAIController>(Pawn->GetController());
		}
		return Cast<AAIController>(Owner);
	}
	return nullptr;
}
