//KSJ : AO_STTask_AIMovementBase

#include "AI/StateTree/Task/AO_STTask_AIMovementBase.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "StateTreeExecutionContext.h"

AAIController* FAO_STTask_AIMovementBase::GetAIController(FStateTreeExecutionContext& Context) const
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

bool FAO_STTask_AIMovementBase::MoveToLocation(AAIController* Controller, const FVector& Location, float AcceptRadius) const
{
	if (!Controller)
	{
		return false;
	}

	if (Location.IsZero())
	{
		return false;
	}

	EPathFollowingRequestResult::Type Result = Controller->MoveToLocation(Location, AcceptRadius);

	return Result != EPathFollowingRequestResult::Failed;
}

EStateTreeRunStatus FAO_STTask_AIMovementBase::CheckMovementStatus(AAIController* Controller) const
{
	if (!Controller)
	{
		return EStateTreeRunStatus::Failed;
	}

	UPathFollowingComponent* PathComp = Controller->GetPathFollowingComponent();
	if (!PathComp)
	{
		return EStateTreeRunStatus::Failed;
	}

	if (PathComp->GetStatus() == EPathFollowingStatus::Idle)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Running;
}

void FAO_STTask_AIMovementBase::StopMovement(AAIController* Controller) const
{
	if (Controller)
	{
		Controller->StopMovement();
	}
}

