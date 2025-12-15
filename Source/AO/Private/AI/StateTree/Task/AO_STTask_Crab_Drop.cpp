// AO_STTask_Crab_Drop.cpp

#include "AI/StateTree/Task/AO_STTask_Crab_Drop.h"
#include "AI/Character/AO_Crab.h"
#include "AI/Component/AO_ItemCarryComponent.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "StateTreeExecutionContext.h"
#include "AO_Log.h"

EStateTreeRunStatus FAO_STTask_Crab_Drop::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FAO_STTask_Crab_Drop_InstanceData& InstanceData = Context.GetInstanceData<FAO_STTask_Crab_Drop_InstanceData>(*this);

	// Crab이 유효해야 드롭 Task 실행 가능
	AAO_Crab* Crab = GetCrab(Context);
	if (!ensureMsgf(Crab, TEXT("Crab_Drop: Crab is null")))
	{
		return EStateTreeRunStatus::Failed;
	}

	// 아이템을 들고 있지 않으면 성공 (할 일 없음)
	if (!Crab->IsCarryingItem())
	{
		return EStateTreeRunStatus::Succeeded;
	}

	// 드롭 위치 계산
	InstanceData.DropLocation = Crab->CalculateItemDropLocation();
	if (InstanceData.DropLocation.IsZero())
	{
		// 위치를 찾지 못하면 현재 위치에 드롭
		if (UAO_ItemCarryComponent* CarryComp = Crab->GetItemCarryComponent())
		{
			CarryComp->DropItem();
		}
		return EStateTreeRunStatus::Succeeded;
	}

	// 드롭 위치로 이동 시작
	if (AAIController* Controller = Cast<AAIController>(Crab->GetController()))
	{
		EPathFollowingRequestResult::Type Result = Controller->MoveToLocation(InstanceData.DropLocation, InstanceData.AcceptanceRadius);

		if (Result == EPathFollowingRequestResult::Failed)
		{
			// 이동 실패 시 현재 위치에 드롭
			if (UAO_ItemCarryComponent* CarryComp = Crab->GetItemCarryComponent())
			{
				CarryComp->DropItem();
			}
			return EStateTreeRunStatus::Succeeded;
		}

		if (Result == EPathFollowingRequestResult::AlreadyAtGoal)
		{
			// 이미 도착 - 바로 드롭
			if (UAO_ItemCarryComponent* CarryComp = Crab->GetItemCarryComponent())
			{
				CarryComp->DropItem();
			}
			return EStateTreeRunStatus::Succeeded;
		}

		InstanceData.bIsMovingToDropLocation = true;
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FAO_STTask_Crab_Drop::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FAO_STTask_Crab_Drop_InstanceData& InstanceData = Context.GetInstanceData<FAO_STTask_Crab_Drop_InstanceData>(*this);

	AAO_Crab* Crab = GetCrab(Context);
	if (!Crab)
	{
		return EStateTreeRunStatus::Failed;
	}

	// 아이템을 더 이상 들고 있지 않으면 성공
	if (!Crab->IsCarryingItem())
	{
		InstanceData.bIsMovingToDropLocation = false;
		return EStateTreeRunStatus::Succeeded;
	}

	// 이동 완료 확인
	if (AAIController* Controller = Cast<AAIController>(Crab->GetController()))
	{
		UPathFollowingComponent* PathComp = Controller->GetPathFollowingComponent();
		if (PathComp && PathComp->GetStatus() == EPathFollowingStatus::Idle)
		{
			// 이동 완료 - 드롭
			if (UAO_ItemCarryComponent* CarryComp = Crab->GetItemCarryComponent())
			{
				CarryComp->DropItem();
			}
			InstanceData.bIsMovingToDropLocation = false;
			return EStateTreeRunStatus::Succeeded;
		}
	}

	return EStateTreeRunStatus::Running;
}

void FAO_STTask_Crab_Drop::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FAO_STTask_Crab_Drop_InstanceData& InstanceData = Context.GetInstanceData<FAO_STTask_Crab_Drop_InstanceData>(*this);

	if (InstanceData.bIsMovingToDropLocation)
	{
		if (AAO_Crab* Crab = GetCrab(Context))
		{
			if (AAIController* Controller = Cast<AAIController>(Crab->GetController()))
			{
				Controller->StopMovement();
			}
		}
	}
	InstanceData.DropLocation = FVector::ZeroVector;
	InstanceData.bIsMovingToDropLocation = false;
}

AAO_Crab* FAO_STTask_Crab_Drop::GetCrab(FStateTreeExecutionContext& Context) const
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
