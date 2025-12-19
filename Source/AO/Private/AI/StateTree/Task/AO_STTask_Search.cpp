// AO_STTask_Search.cpp

#include "AI/StateTree/Task/AO_STTask_Search.h"
#include "AI/Base/AO_AggressiveAIBase.h"
#include "AI/Controller/AO_AggressiveAICtrl.h"
#include "StateTreeExecutionContext.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "AO_Log.h"

EStateTreeRunStatus FAO_STTask_Search::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FAO_STTask_Search_InstanceData& InstanceData = Context.GetInstanceData<FAO_STTask_Search_InstanceData>(*this);

	AAO_AggressiveAICtrl* Controller = GetAggressiveController(Context);
	if (!Controller)
	{
		return EStateTreeRunStatus::Failed;
	}

	// 마지막으로 플레이어를 본 위치를 수색 중심으로
	InstanceData.SearchCenterLocation = Controller->GetLastKnownTargetLocation();
	
	// 유효하지 않은 위치면 현재 위치 사용
	if (InstanceData.SearchCenterLocation.IsNearlyZero())
	{
		if (APawn* Pawn = Controller->GetPawn())
		{
			InstanceData.SearchCenterLocation = Pawn->GetActorLocation();
		}
	}

	// 수색 시간 설정 (5~10초 랜덤)
	InstanceData.SearchDuration = FMath::RandRange(5.f, 10.f);
	InstanceData.RemainingSearchTime = InstanceData.SearchDuration;
	InstanceData.bIsMoving = false;

	// 첫 수색 위치로 이동 시작
	const FVector FirstSearchLocation = FindRandomSearchLocation(Context, InstanceData.SearchCenterLocation, InstanceData.SearchRadius);
	if (!FirstSearchLocation.IsNearlyZero())
	{
		if (MoveToLocation(Controller, FirstSearchLocation, InstanceData.AcceptanceRadius))
		{
			InstanceData.bIsMoving = true;
		}
	}

	AO_LOG(LogKSJ, Log, TEXT("Search started at %.0f, %.0f, %.0f for %.1f seconds"),
		InstanceData.SearchCenterLocation.X, InstanceData.SearchCenterLocation.Y, InstanceData.SearchCenterLocation.Z,
		InstanceData.SearchDuration);

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FAO_STTask_Search::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FAO_STTask_Search_InstanceData& InstanceData = Context.GetInstanceData<FAO_STTask_Search_InstanceData>(*this);

	AAO_AggressiveAICtrl* Controller = GetAggressiveController(Context);
	if (!Controller)
	{
		return EStateTreeRunStatus::Failed;
	}

	AAO_AggressiveAIBase* AI = Controller->GetAggressiveAI();
	if (!AI)
	{
		return EStateTreeRunStatus::Failed;
	}

	// 기절 상태면 중단
	if (AI->IsStunned())
	{
		return EStateTreeRunStatus::Failed;
	}

	// 시야에 플레이어가 다시 들어오면 성공 (추격으로 전환)
	if (Controller->HasPlayerInSight())
	{
		return EStateTreeRunStatus::Succeeded;
	}

	// 수색 시간 감소
	InstanceData.RemainingSearchTime -= DeltaTime;
	if (InstanceData.RemainingSearchTime <= 0.f)
	{
		// 수색 시간 종료 - 배회로 전환
		Controller->EndSearch();
		return EStateTreeRunStatus::Failed;
	}

	// 현재 이동이 완료되었는지 확인
	const EPathFollowingStatus::Type MoveStatus = Controller->GetMoveStatus();
	if (MoveStatus != EPathFollowingStatus::Moving)
	{
		// 다음 수색 위치로 이동
		const FVector NextSearchLocation = FindRandomSearchLocation(Context, InstanceData.SearchCenterLocation, InstanceData.SearchRadius);
		if (!NextSearchLocation.IsNearlyZero())
		{
			MoveToLocation(Controller, NextSearchLocation, InstanceData.AcceptanceRadius);
		}
	}

	return EStateTreeRunStatus::Running;
}

void FAO_STTask_Search::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FAO_STTask_Search_InstanceData& InstanceData = Context.GetInstanceData<FAO_STTask_Search_InstanceData>(*this);

	if (InstanceData.bIsMoving)
	{
		AAO_AggressiveAICtrl* Controller = GetAggressiveController(Context);
		if (Controller)
		{
			Controller->StopMovement();
		}
		InstanceData.bIsMoving = false;
	}
}

AAO_AggressiveAICtrl* FAO_STTask_Search::GetAggressiveController(FStateTreeExecutionContext& Context) const
{
	if (AActor* Owner = Cast<AActor>(Context.GetOwner()))
	{
		if (AAO_AggressiveAICtrl* Controller = Cast<AAO_AggressiveAICtrl>(Owner))
		{
			return Controller;
		}
		if (APawn* Pawn = Cast<APawn>(Owner))
		{
			return Cast<AAO_AggressiveAICtrl>(Pawn->GetController());
		}
	}
	return nullptr;
}

FVector FAO_STTask_Search::FindRandomSearchLocation(FStateTreeExecutionContext& Context, const FVector& Center, float Radius) const
{
	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(Context.GetWorld());
	if (!NavSystem)
	{
		return FVector::ZeroVector;
	}

	FNavLocation OutLocation;
	const bool bFound = NavSystem->GetRandomReachablePointInRadius(Center, Radius, OutLocation);

	return bFound ? OutLocation.Location : FVector::ZeroVector;
}

bool FAO_STTask_Search::MoveToLocation(AAO_AggressiveAICtrl* Controller, const FVector& Location, float AcceptRadius) const
{
	if (!Controller)
	{
		return false;
	}

	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalLocation(Location);
	MoveRequest.SetAcceptanceRadius(AcceptRadius);
	MoveRequest.SetUsePathfinding(true);

	const FPathFollowingRequestResult Result = Controller->MoveTo(MoveRequest);
	return Result.Code != EPathFollowingRequestResult::Failed;
}

