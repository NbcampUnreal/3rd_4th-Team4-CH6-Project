// AO_STTask_Crab_Drop.cpp

#include "AI/StateTree/Task/AO_STTask_Crab_Drop.h"
#include "AI/Character/AO_Crab.h"
#include "AI/Controller/AO_CrabController.h"
#include "AI/Component/AO_ItemCarryComponent.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "StateTreeExecutionContext.h"

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

	// 플레이어가 근처에 있으면 즉시 Flee로 전환 (도망이 우선)
	if (AAO_CrabController* Controller = Cast<AAO_CrabController>(Crab->GetController()))
	{
		// 시야 내 플레이어가 있으면 즉시 실패하여 Flee로 전환
		if (Controller->HasPlayerInSight())
		{
			return EStateTreeRunStatus::Failed;
		}

		// 플레이어가 너무 가까이 있으면 (위협 위치 기준) 즉시 실패
		FVector ThreatLocation = Controller->GetNearestThreatLocation();
		if (!ThreatLocation.IsZero())
		{
			float DistToThreat = FVector::Dist(Crab->GetActorLocation(), ThreatLocation);
			if (DistToThreat < 1000.f) // 안전 거리 이내에 플레이어가 있으면 도망
			{
				return EStateTreeRunStatus::Failed;
			}
		}
	}

	// 드롭 위치 계산 (이전에 실패한 위치는 제외)
	// InstanceData.DropLocation이 이전 시도 위치를 담고 있을 수 있음 (ExitState에서 초기화되지 않은 경우)
	FVector PreviousDropLocation = InstanceData.DropLocation;
	InstanceData.DropLocation = Crab->CalculateItemDropLocation(PreviousDropLocation);
	
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

	// 플레이어 감지 및 드롭 위치 재계산
	if (AAO_CrabController* Controller = Cast<AAO_CrabController>(Crab->GetController()))
	{
		// 플레이어가 시야에 있으면 즉시 Flee로 전환
		if (Controller->HasPlayerInSight())
		{
			return EStateTreeRunStatus::Failed;
		}

		// 플레이어가 근처에 있으면 드롭 위치 재계산
		FVector ThreatLocation = Controller->GetNearestThreatLocation();
		if (!ThreatLocation.IsZero())
		{
			// 1. 내 몸이 위험한 경우
			const float DistToThreat = FVector::Dist(Crab->GetActorLocation(), ThreatLocation);
			
			// 2. 목적지가 위험해진 경우 (플레이어가 이동해서)
			const float DistDestToThreat = FVector::Dist(InstanceData.DropLocation, ThreatLocation);
			const float MinSafeDestDist = 1200.f; // 목적지 안전 마진

			if (DistToThreat < 800.f || DistDestToThreat < MinSafeDestDist)
			{
				// 현재 목적지를 제외하고 새 위치 계산 (같은 곳으로 다시 가지 않도록)
				FVector NewDropLocation = Crab->CalculateItemDropLocation(InstanceData.DropLocation);
				
				// 새 위치가 유효하고, 기존과 다르며
				if (!NewDropLocation.IsZero() && NewDropLocation != InstanceData.DropLocation)
				{
					// 새로 찾은 곳도 여전히 위험하다면(갈 곳이 없다면) Task 실패 -> Flee 유도
					if (FVector::Dist(NewDropLocation, ThreatLocation) < MinSafeDestDist)
					{
						return EStateTreeRunStatus::Failed;
					}

					InstanceData.DropLocation = NewDropLocation;
					Controller->MoveToLocation(InstanceData.DropLocation, InstanceData.AcceptanceRadius);
				}
				else
				{
					// 새 위치를 찾지 못했으면 Flee로 전환
					return EStateTreeRunStatus::Failed;
				}
			}
		}
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
	
	// 성공적으로 완료된 경우에만 DropLocation 초기화
	// 실패(Flee로 전환)된 경우에는 이전 위치를 유지하여 재진입 시 다른 위치 선택 유도
	if (Transition.CurrentRunStatus == EStateTreeRunStatus::Succeeded)
	{
		InstanceData.DropLocation = FVector::ZeroVector;
	}
	// 실패 시에는 DropLocation을 유지 (다음 EnterState에서 ExcludeLocation으로 사용됨)
	
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
