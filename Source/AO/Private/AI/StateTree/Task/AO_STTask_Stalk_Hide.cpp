// AO_STTask_Stalk_Hide.cpp

#include "AI/StateTree/Task/AO_STTask_Stalk_Hide.h"
#include "AI/Controller/AO_StalkerController.h"
#include "AI/Character/AO_Stalker.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "StateTreeExecutionContext.h"
#include "Navigation/PathFollowingComponent.h"

EStateTreeRunStatus FAO_STTask_Stalk_Hide::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	
	AActor* Owner = Cast<AActor>(Context.GetOwner());
	if (AAO_StalkerController* Ctrl = Cast<AAO_StalkerController>(Owner ? Owner->GetInstigatorController() : nullptr))
	{
		InstanceData.Controller = Ctrl;
	}
	else if (APawn* Pawn = Cast<APawn>(Owner))
	{
		InstanceData.Controller = Cast<AAO_StalkerController>(Pawn->GetController());
	}

	if (!InstanceData.Controller)
	{
		return EStateTreeRunStatus::Failed;
	}

	// EQS 실행하여 숨을 곳 찾기
	if (InstanceData.HideQuery)
	{
		FEnvQueryRequest Request(InstanceData.HideQuery, InstanceData.Controller->GetPawn());
		Request.Execute(EEnvQueryRunMode::SingleResult, 
			FQueryFinishedSignature::CreateWeakLambda(InstanceData.Controller, 
			[Controller = InstanceData.Controller, &InstanceData](TSharedPtr<FEnvQueryResult> Result)
			{
				if (Result.IsValid() && Result->IsSuccessful() && Controller)
				{
					FVector HideLoc = Result->GetItemAsLocation(0);
					Controller->MoveToLocation(HideLoc);
					InstanceData.bIsMoving = true;
				}
			}));
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FAO_STTask_Stalk_Hide::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	if (!InstanceData.Controller) return EStateTreeRunStatus::Failed;

	// 이동 완료 확인
	if (InstanceData.bIsMoving)
	{
		if (InstanceData.Controller->GetMoveStatus() == EPathFollowingStatus::Idle)
		{
			// 숨기 완료
			InstanceData.bIsMoving = false;
			return EStateTreeRunStatus::Succeeded;
		}
	}

	return EStateTreeRunStatus::Running;
}

void FAO_STTask_Stalk_Hide::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	if (InstanceData.Controller)
	{
		InstanceData.Controller->StopMovement();
	}
}

