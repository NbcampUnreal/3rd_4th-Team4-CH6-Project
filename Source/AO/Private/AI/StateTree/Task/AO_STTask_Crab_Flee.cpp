// AO_STTask_Crab_Flee.cpp

#include "AI/StateTree/Task/AO_STTask_Crab_Flee.h"
#include "AI/Character/AO_Crab.h"
#include "AI/Controller/AO_CrabController.h"
#include "AI/Component/AO_ItemCarryComponent.h"
#include "Character/AO_PlayerCharacter.h"
#include "Navigation/PathFollowingComponent.h"
#include "StateTreeExecutionContext.h"
#include "AO_Log.h"

EStateTreeRunStatus FAO_STTask_Crab_Flee::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FAO_STTask_Crab_Flee_InstanceData& InstanceData = Context.GetInstanceData<FAO_STTask_Crab_Flee_InstanceData>(*this);

	// Crab이 유효해야 도주 Task 실행 가능
	AAO_Crab* Crab = GetCrab(Context);
	if (!ensureMsgf(Crab, TEXT("Crab_Flee: Crab is null")))
	{
		return EStateTreeRunStatus::Failed;
	}

	// Controller가 유효해야 도주 위치 계산 및 이동 가능
	AAO_CrabController* Controller = Cast<AAO_CrabController>(Crab->GetController());
	if (!ensureMsgf(Controller, TEXT("Crab_Flee: CrabController is null")))
	{
		return EStateTreeRunStatus::Failed;
	}

	// 도주 모드 활성화
	Crab->SetFleeMode(true);

	// 현재 위협 위치 기억 (도주 시작 시점의 플레이어 위치)
	InstanceData.LastThreatLocation = Controller->GetNearestThreatLocation();
	if (InstanceData.LastThreatLocation.IsZero())
	{
		// 시야 내 플레이어 위치 사용
		if (AAO_PlayerCharacter* Player = Controller->GetNearestPlayerInSight())
		{
			InstanceData.LastThreatLocation = Player->GetActorLocation();
		}
	}

	// 도주 위치 계산
	InstanceData.FleeLocation = Controller->CalculateFleeLocation();
	if (InstanceData.FleeLocation.IsZero())
	{
		return EStateTreeRunStatus::Failed;
	}

	// 도주 위치로 이동
	EPathFollowingRequestResult::Type Result = Controller->MoveToLocation(InstanceData.FleeLocation, InstanceData.AcceptanceRadius);
	if (Result == EPathFollowingRequestResult::Failed)
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.bIsFleeing = true;
	InstanceData.RecalculateTimer = 0.f;

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FAO_STTask_Crab_Flee::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FAO_STTask_Crab_Flee_InstanceData& InstanceData = Context.GetInstanceData<FAO_STTask_Crab_Flee_InstanceData>(*this);

	AAO_Crab* Crab = GetCrab(Context);
	if (!Crab)
	{
		return EStateTreeRunStatus::Failed;
	}

	AAO_CrabController* Controller = Cast<AAO_CrabController>(Crab->GetController());
	if (!Controller)
	{
		return EStateTreeRunStatus::Failed;
	}

	// 시야 내 플레이어가 있으면 위협 위치 갱신
	if (Controller->HasPlayerInSight())
	{
		FVector NewThreatLocation = Controller->GetNearestThreatLocation();
		if (!NewThreatLocation.IsZero())
		{
			InstanceData.LastThreatLocation = NewThreatLocation;
		}
	}

	// 마지막 위협 위치로부터 충분히 멀어졌는지 확인
	if (!InstanceData.LastThreatLocation.IsZero())
	{
		float DistFromThreat = FVector::Dist(Crab->GetActorLocation(), InstanceData.LastThreatLocation);
		
		// 시야 내 플레이어가 없고, 안전 거리만큼 멀어졌으면 도주 종료
		if (!Controller->HasPlayerInSight() && DistFromThreat >= InstanceData.SafeDistance)
		{
			Crab->SetFleeMode(false);
			InstanceData.bIsFleeing = false;
			return EStateTreeRunStatus::Succeeded;
		}
	}

	// 주기적으로 도주 위치 재계산
	InstanceData.RecalculateTimer += DeltaTime;
	if (InstanceData.RecalculateTimer >= InstanceData.RecalculateInterval)
	{
		InstanceData.RecalculateTimer = 0.f;
		FVector NewFleeLocation = Controller->CalculateFleeLocation();
		if (!NewFleeLocation.IsZero())
		{
			InstanceData.FleeLocation = NewFleeLocation;
			Controller->MoveToLocation(InstanceData.FleeLocation, InstanceData.AcceptanceRadius);
		}
	}

	return EStateTreeRunStatus::Running;
}

void FAO_STTask_Crab_Flee::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FAO_STTask_Crab_Flee_InstanceData& InstanceData = Context.GetInstanceData<FAO_STTask_Crab_Flee_InstanceData>(*this);

	if (AAO_Crab* Crab = GetCrab(Context))
	{
		Crab->SetFleeMode(false);

		if (InstanceData.bIsFleeing)
		{
			if (AAIController* Controller = Cast<AAIController>(Crab->GetController()))
			{
				Controller->StopMovement();
			}
		}
	}

	InstanceData.FleeLocation = FVector::ZeroVector;
	InstanceData.RecalculateTimer = 0.f;
	InstanceData.bIsFleeing = false;
}

AAO_Crab* FAO_STTask_Crab_Flee::GetCrab(FStateTreeExecutionContext& Context) const
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

bool FAO_STTask_Crab_Flee::IsPlayerNearby(FStateTreeExecutionContext& Context, float SafeDist) const
{
	AAO_Crab* Crab = GetCrab(Context);
	if (!Crab)
	{
		return false;
	}

	AAO_CrabController* Controller = Cast<AAO_CrabController>(Crab->GetController());
	if (!Controller)
	{
		return false;
	}

	// 시야 내 플레이어가 있으면 근처에 있음
	if (Controller->HasPlayerInSight())
	{
		return true;
	}

	// 가장 가까운 위협 위치 확인
	FVector ThreatLocation = Controller->GetNearestThreatLocation();
	if (!ThreatLocation.IsZero())
	{
		float Distance = FVector::Dist(Crab->GetActorLocation(), ThreatLocation);
		return Distance < SafeDist;
	}

	return false;
}
