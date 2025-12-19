// AO_STTask_Crab_Pickup.cpp

#include "AI/StateTree/Task/AO_STTask_Crab_Pickup.h"
#include "AI/Character/AO_Crab.h"
#include "AI/Component/AO_ItemCarryComponent.h"
#include "AI/Controller/AO_CrabController.h"
#include "Item/AO_MasterItem.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "StateTreeExecutionContext.h"
#include "AO_Log.h"

EStateTreeRunStatus FAO_STTask_Crab_Pickup::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FAO_STTask_Crab_Pickup_InstanceData& InstanceData = Context.GetInstanceData<FAO_STTask_Crab_Pickup_InstanceData>(*this);

	// Crab이 유효해야 픽업 Task 실행 가능
	AAO_Crab* Crab = GetCrab(Context);
	if (!ensureMsgf(Crab, TEXT("Crab_Pickup: Crab is null")))
	{
		return EStateTreeRunStatus::Failed;
	}

	// 이미 아이템을 들고 있으면 성공
	if (Crab->IsCarryingItem())
	{
		return EStateTreeRunStatus::Succeeded;
	}

	// 주변 아이템 탐색
	InstanceData.TargetItem = FindNearbyItem(Context, InstanceData.SearchRadius);
	if (!InstanceData.TargetItem.IsValid())
	{
		return EStateTreeRunStatus::Failed;
	}

	// 아이템으로 이동 시작
	if (AAO_CrabController* Controller = Cast<AAO_CrabController>(Crab->GetController()))
	{
		EPathFollowingRequestResult::Type Result = Controller->MoveToActor(InstanceData.TargetItem.Get(), InstanceData.PickupDistance);
		if (Result == EPathFollowingRequestResult::Failed)
		{
			InstanceData.TargetItem.Reset();
			return EStateTreeRunStatus::Failed;
		}

		if (Result == EPathFollowingRequestResult::AlreadyAtGoal)
		{
			// 이미 가까이 있음 - 바로 줍기 시도
			if (UAO_ItemCarryComponent* CarryComp = Crab->GetItemCarryComponent())
			{
				if (CarryComp->TryPickupItem(InstanceData.TargetItem.Get()))
				{
					InstanceData.TargetItem.Reset();
					return EStateTreeRunStatus::Succeeded;
				}
			}
			InstanceData.TargetItem.Reset();
			return EStateTreeRunStatus::Failed;
		}

		InstanceData.bIsMovingToItem = true;
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FAO_STTask_Crab_Pickup::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FAO_STTask_Crab_Pickup_InstanceData& InstanceData = Context.GetInstanceData<FAO_STTask_Crab_Pickup_InstanceData>(*this);

	AAO_Crab* Crab = GetCrab(Context);
	if (!Crab)
	{
		return EStateTreeRunStatus::Failed;
	}

	// 타겟 아이템이 유효하지 않으면 실패
	if (!InstanceData.TargetItem.IsValid())
	{
		return EStateTreeRunStatus::Failed;
	}

	// 이미 아이템을 들고 있으면 성공
	if (Crab->IsCarryingItem())
	{
		return EStateTreeRunStatus::Succeeded;
	}

	// 이동 완료 확인
	if (AAIController* Controller = Cast<AAIController>(Crab->GetController()))
	{
		UPathFollowingComponent* PathComp = Controller->GetPathFollowingComponent();
		if (PathComp && PathComp->GetStatus() == EPathFollowingStatus::Idle)
		{
			// 이동 완료 - 줍기 시도
			if (UAO_ItemCarryComponent* CarryComp = Crab->GetItemCarryComponent())
			{
				if (CarryComp->TryPickupItem(InstanceData.TargetItem.Get()))
				{
					InstanceData.TargetItem.Reset();
					InstanceData.bIsMovingToItem = false;
					return EStateTreeRunStatus::Succeeded;
				}
			}
			// 줍기 실패
			InstanceData.TargetItem.Reset();
			InstanceData.bIsMovingToItem = false;
			return EStateTreeRunStatus::Failed;
		}
	}

	return EStateTreeRunStatus::Running;
}

void FAO_STTask_Crab_Pickup::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FAO_STTask_Crab_Pickup_InstanceData& InstanceData = Context.GetInstanceData<FAO_STTask_Crab_Pickup_InstanceData>(*this);

	if (InstanceData.bIsMovingToItem)
	{
		if (AAO_Crab* Crab = GetCrab(Context))
		{
			if (AAIController* Controller = Cast<AAIController>(Crab->GetController()))
			{
				Controller->StopMovement();
			}
		}
	}
	InstanceData.TargetItem.Reset();
	InstanceData.bIsMovingToItem = false;
}

AAO_Crab* FAO_STTask_Crab_Pickup::GetCrab(FStateTreeExecutionContext& Context) const
{
	if (AActor* Owner = Cast<AActor>(Context.GetOwner()))
	{
		if (AAO_Crab* Crab = Cast<AAO_Crab>(Owner))
		{
			return Crab;
		}
		if (AAIController* Controller = Cast<AAIController>(Owner))
		{
			return Cast<AAO_Crab>(Controller->GetPawn());
		}
	}
	return nullptr;
}

AAO_MasterItem* FAO_STTask_Crab_Pickup::FindNearbyItem(FStateTreeExecutionContext& Context, float Radius) const
{
	AAO_Crab* Crab = GetCrab(Context);
	if (!Crab)
	{
		return nullptr;
	}

	if (UAO_ItemCarryComponent* CarryComp = Crab->GetItemCarryComponent())
	{
		return CarryComp->FindNearbyItem(Radius);
	}

	return nullptr;
}
