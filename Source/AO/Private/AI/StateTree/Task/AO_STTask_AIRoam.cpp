// AO_STTask_AIRoam.cpp

#include "AI/StateTree/Task/AO_STTask_AIRoam.h"
#include "AI/Character/AO_Crab.h"
#include "AI/Character/AO_Stalker.h"
#include "AI/Component/AO_ItemCarryComponent.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "StateTreeExecutionContext.h"
#include "AO_Log.h"
#include "AI/Component/AO_CeilingMoveComponent.h"

EStateTreeRunStatus FAO_STTask_AIRoam::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FAO_STTask_AIRoam_InstanceData& InstanceData = Context.GetInstanceData<FAO_STTask_AIRoam_InstanceData>(*this);

	// AIController가 유효해야 배회 가능
	AAIController* AIController = GetAIController(Context);
	if (!ensureMsgf(AIController, TEXT("AIRoam: AIController is null")))
	{
		return EStateTreeRunStatus::Failed;
	}

	// 랜덤 배회 위치 찾기
	FVector RoamLocation = FindRandomRoamLocation(Context, InstanceData.RoamRadius);
	if (RoamLocation.IsZero())
	{
		AO_LOG(LogKSJ, Warning, TEXT("AIRoam: Failed to find roam location"));
		return EStateTreeRunStatus::Failed;
	}

	// 배회 위치로 이동 시작
	if (!StartMovingToLocation(AIController, RoamLocation, InstanceData.AcceptanceRadius))
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.bIsMoving = true;
	InstanceData.bIsWaiting = false;
	InstanceData.WaitTimer = 0.f;

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FAO_STTask_AIRoam::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FAO_STTask_AIRoam_InstanceData& InstanceData = Context.GetInstanceData<FAO_STTask_AIRoam_InstanceData>(*this);

	AAIController* AIController = GetAIController(Context);
	if (!AIController)
	{
		return EStateTreeRunStatus::Failed;
	}

	// Crab인 경우 아이템 발견 체크 (아이템 발견 시 PickupItem State로 전환되도록 실패 반환)
	// Stalker인 경우 천장 감지 및 자동 전환
	if (APawn* Pawn = AIController->GetPawn())
	{
		if (AAO_Crab* Crab = Cast<AAO_Crab>(Pawn))
		{
			if (UAO_ItemCarryComponent* CarryComp = Crab->GetItemCarryComponent())
			{
				// 주변에 아이템이 있으면 실패 반환하여 다른 State로 전환되도록 함
				AAO_MasterItem* NearbyItem = CarryComp->FindNearbyItem(1500.f);
				if (NearbyItem)
				{
					return EStateTreeRunStatus::Failed;
				}
			}
		}
		else if (AAO_Stalker* Stalker = Cast<AAO_Stalker>(Pawn))
		{
			// Stalker: 천장이 있으면 자동으로 천장 모드로 전환
			if (UAO_CeilingMoveComponent* CeilingComp = Stalker->GetCeilingMoveComponent())
			{
				if (!CeilingComp->IsInCeilingMode() && CeilingComp->CheckCeilingAvailability())
				{
					Stalker->SetCeilingMode(true);
					AO_LOG(LogKSJ, Log, TEXT("Stalker: Auto-enabled ceiling mode during Roam"));
				}
			}
		}
	}

	// 대기 중인 경우
	if (InstanceData.bIsWaiting)
	{
		InstanceData.WaitTimer += DeltaTime;
		if (InstanceData.WaitTimer >= InstanceData.WaitTimeAtDestination)
		{
			// 새로운 배회 위치로 이동
			FVector NewLocation = FindRandomRoamLocation(Context, InstanceData.RoamRadius);
			if (!NewLocation.IsZero() && StartMovingToLocation(AIController, NewLocation, InstanceData.AcceptanceRadius))
			{
				InstanceData.bIsWaiting = false;
				InstanceData.bIsMoving = true;
				InstanceData.WaitTimer = 0.f;
			}
		}
		return EStateTreeRunStatus::Running;
	}

	// 이동 중인 경우
	if (InstanceData.bIsMoving)
	{
		UPathFollowingComponent* PathComp = AIController->GetPathFollowingComponent();
		if (PathComp && PathComp->GetStatus() == EPathFollowingStatus::Idle)
		{
			// 목적지 도착 - 대기 시작
			InstanceData.bIsMoving = false;
			InstanceData.bIsWaiting = true;
			InstanceData.WaitTimer = 0.f;
		}
	}

	return EStateTreeRunStatus::Running;
}

void FAO_STTask_AIRoam::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FAO_STTask_AIRoam_InstanceData& InstanceData = Context.GetInstanceData<FAO_STTask_AIRoam_InstanceData>(*this);

	if (InstanceData.bIsMoving)
	{
		if (AAIController* AIController = GetAIController(Context))
		{
			AIController->StopMovement();
		}
	}
	InstanceData.bIsMoving = false;
	InstanceData.bIsWaiting = false;
	InstanceData.WaitTimer = 0.f;
}

AAIController* FAO_STTask_AIRoam::GetAIController(FStateTreeExecutionContext& Context) const
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

FVector FAO_STTask_AIRoam::FindRandomRoamLocation(FStateTreeExecutionContext& Context, float Radius) const
{
	AAIController* AIController = GetAIController(Context);
	if (!AIController || !AIController->GetPawn())
	{
		return FVector::ZeroVector;
	}

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(AIController->GetWorld());
	if (!NavSys)
	{
		return FVector::ZeroVector;
	}

	FNavLocation NavLocation;
	const FVector Origin = AIController->GetPawn()->GetActorLocation();

	if (NavSys->GetRandomReachablePointInRadius(Origin, Radius, NavLocation))
	{
		return NavLocation.Location;
	}

	return FVector::ZeroVector;
}

bool FAO_STTask_AIRoam::StartMovingToLocation(AAIController* AIController, const FVector& Location, float AcceptRadius) const
{
	if (!AIController)
	{
		return false;
	}

	EPathFollowingRequestResult::Type Result = AIController->MoveToLocation(Location, AcceptRadius);
	return Result != EPathFollowingRequestResult::Failed;
}
