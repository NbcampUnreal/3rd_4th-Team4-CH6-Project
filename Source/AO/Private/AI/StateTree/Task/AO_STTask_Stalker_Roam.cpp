//KSJ : AO_STTask_Stalker_Roam

#include "AI/StateTree/Task/AO_STTask_Stalker_Roam.h"
#include "AI/Character/AO_Stalker.h"
#include "AI/Component/AO_CeilingMoveComponent.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "StateTreeExecutionContext.h"

EStateTreeRunStatus FAO_STTask_Stalker_Roam::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FAO_STTask_Stalker_Roam_InstanceData& InstanceData = Context.GetInstanceData<FAO_STTask_Stalker_Roam_InstanceData>(*this);

	AAIController* AIController = GetAIController(Context);
	if (!ensureMsgf(AIController, TEXT("StalkerRoam: AIController is null")))
	{
		return EStateTreeRunStatus::Failed;
	}

	// Stalker 및 컴포넌트 캐싱
	if (InstanceData.Stalker = Cast<AAO_Stalker>(AIController->GetPawn()))
	{
		InstanceData.CeilingComp = InstanceData.Stalker->GetCeilingMoveComponent();
	}

	// 첫 이동 시작
	InstanceData.PendingRoamLocation = FindRandomRoamLocation(Context, InstanceData.RoamRadius);
	if (InstanceData.PendingRoamLocation.IsZero())
	{
		return EStateTreeRunStatus::Failed;
	}

	// 이동 시작 시 천장 모드 결정 (운 좋고 천장 있으면 올라감)
	if (InstanceData.CeilingComp && InstanceData.Stalker)
	{
		bool bCanCeiling = InstanceData.CeilingComp->CheckCeilingAvailability();
		bool bWantCeiling = FMath::FRand() <= InstanceData.CeilingMoveChance;
		
		// 몽타주를 통한 전환 시도
		if (bCanCeiling && bWantCeiling)
		{
			InstanceData.Stalker->PlayCeilingTransitionMontage(true);
			if (InstanceData.Stalker->IsTransitioningCeiling())
			{
				InstanceData.bWaitingForTransition = true;
			}
		}
	}

	// 전환 몽타주 중이면, 종료 후 이동을 시작한다.
	if (InstanceData.bWaitingForTransition)
	{
		InstanceData.bIsMoving = false;
		InstanceData.bIsWaiting = false;
		InstanceData.WaitTimer = 0.f;
		return EStateTreeRunStatus::Running;
	}

	if (!StartMovingToLocation(AIController, InstanceData.PendingRoamLocation, InstanceData.AcceptanceRadius))
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.bIsMoving = true;
	InstanceData.bIsWaiting = false;
	InstanceData.WaitTimer = 0.f;

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FAO_STTask_Stalker_Roam::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FAO_STTask_Stalker_Roam_InstanceData& InstanceData = Context.GetInstanceData<FAO_STTask_Stalker_Roam_InstanceData>(*this);

	AAIController* AIController = GetAIController(Context);
	if (!AIController)
	{
		return EStateTreeRunStatus::Failed;
	}

	// 전환 대기 중: 몽타주가 끝나면 이동을 시작한다.
	if (InstanceData.bWaitingForTransition && InstanceData.Stalker)
	{
		if (InstanceData.Stalker->IsTransitioningCeiling())
		{
			return EStateTreeRunStatus::Running;
		}

		// 전환 완료 -> 이동 시작
		InstanceData.bWaitingForTransition = false;
		if (!InstanceData.PendingRoamLocation.IsZero())
		{
			if (!StartMovingToLocation(AIController, InstanceData.PendingRoamLocation, InstanceData.AcceptanceRadius))
			{
				return EStateTreeRunStatus::Failed;
			}
			InstanceData.bIsMoving = true;
			InstanceData.bIsWaiting = false;
			InstanceData.WaitTimer = 0.f;
		}
	}

	// 천장 안전 장치: 천장이 없는데 천장 모드라면 내려오기
	if (InstanceData.CeilingComp && InstanceData.CeilingComp->IsInCeilingMode() && InstanceData.Stalker)
	{
		if (!InstanceData.CeilingComp->CheckCeilingAvailability())
		{
			// 천장 끊김 -> 바닥으로 복귀 (몽타주 재생)
			InstanceData.Stalker->PlayCeilingTransitionMontage(false);
			if (InstanceData.Stalker->IsTransitioningCeiling())
			{
				InstanceData.bWaitingForTransition = true;
				InstanceData.bIsMoving = false;
				InstanceData.bIsWaiting = false;
				return EStateTreeRunStatus::Running;
			}
		}
	}

	// 대기 중 처리
	if (InstanceData.bIsWaiting)
	{
		InstanceData.WaitTimer += DeltaTime;
		if (InstanceData.WaitTimer >= InstanceData.WaitTimeAtDestination)
		{
			// 새로운 이동 시작
			InstanceData.PendingRoamLocation = FindRandomRoamLocation(Context, InstanceData.RoamRadius);
			
			// 다음 이동을 위해 천장 모드 재설정 (확률)
			if (InstanceData.CeilingComp && InstanceData.Stalker)
			{
				bool bCanCeiling = InstanceData.CeilingComp->CheckCeilingAvailability();
				bool bWantCeiling = FMath::FRand() <= InstanceData.CeilingMoveChance;
				
				// 이미 천장에 있다면 유지할지, 내려올지 결정
				// 바닥에 있다면 올라갈지 결정
				if (InstanceData.CeilingComp->IsInCeilingMode())
				{
					// 천장에 있는데 천장 없거나 운 나쁘면 내려옴
					if (!bCanCeiling || !bWantCeiling)
					{
						InstanceData.Stalker->PlayCeilingTransitionMontage(false);
						if (InstanceData.Stalker->IsTransitioningCeiling())
						{
							InstanceData.bWaitingForTransition = true;
						}
					}
				}
				else
				{
					// 바닥에 있는데 천장 있고 운 좋으면 올라감
					if (bCanCeiling && bWantCeiling)
					{
						InstanceData.Stalker->PlayCeilingTransitionMontage(true);
						if (InstanceData.Stalker->IsTransitioningCeiling())
						{
							InstanceData.bWaitingForTransition = true;
						}
					}
				}
			}

			// 전환 몽타주 중이면, 종료 후 이동을 시작한다.
			if (InstanceData.bWaitingForTransition)
			{
				InstanceData.bIsWaiting = false;
				InstanceData.bIsMoving = false;
				InstanceData.WaitTimer = 0.f;
				return EStateTreeRunStatus::Running;
			}

			if (!InstanceData.PendingRoamLocation.IsZero() && StartMovingToLocation(AIController, InstanceData.PendingRoamLocation, InstanceData.AcceptanceRadius))
			{
				InstanceData.bIsWaiting = false;
				InstanceData.bIsMoving = true;
				InstanceData.WaitTimer = 0.f;
			}
		}
		return EStateTreeRunStatus::Running;
	}

	// 이동 중 처리
	if (InstanceData.bIsMoving)
	{
		UPathFollowingComponent* PathComp = AIController->GetPathFollowingComponent();
		if (PathComp && PathComp->GetStatus() == EPathFollowingStatus::Idle)
		{
			InstanceData.bIsMoving = false;
			InstanceData.bIsWaiting = true;
			InstanceData.WaitTimer = 0.f;
		}
	}

	return EStateTreeRunStatus::Running;
}

void FAO_STTask_Stalker_Roam::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FAO_STTask_Stalker_Roam_InstanceData& InstanceData = Context.GetInstanceData<FAO_STTask_Stalker_Roam_InstanceData>(*this);

	if (InstanceData.bIsMoving)
	{
		if (AAIController* AIController = GetAIController(Context))
		{
			AIController->StopMovement();
		}
	}
	InstanceData.bIsMoving = false;
	InstanceData.bIsWaiting = false;
	
	// Exit 시 천장 모드를 굳이 해제하지 않음 (자연스러운 전환을 위해)
	// 다음 State(Approach, Hide 등)의 EnterState에서 필요시 해제함.
}

AAIController* FAO_STTask_Stalker_Roam::GetAIController(FStateTreeExecutionContext& Context) const
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

FVector FAO_STTask_Stalker_Roam::FindRandomRoamLocation(FStateTreeExecutionContext& Context, float Radius) const
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

bool FAO_STTask_Stalker_Roam::StartMovingToLocation(AAIController* AIController, const FVector& Location, float AcceptRadius) const
{
	if (!AIController) return false;
	EPathFollowingRequestResult::Type Result = AIController->MoveToLocation(Location, AcceptRadius);
	return Result != EPathFollowingRequestResult::Failed;
}
