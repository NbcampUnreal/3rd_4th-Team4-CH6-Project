//KSJ : AO_STTask_Chase

#include "AI/StateTree/Task/AO_STTask_Chase.h"
#include "AI/Base/AO_AggressiveAIBase.h"
#include "AI/Character/AO_Stalker.h"
#include "AI/Controller/AO_AggressiveAICtrl.h"
#include "AI/Controller/AO_AIControllerBase.h"
#include "Character/AO_PlayerCharacter.h"
#include "StateTreeExecutionContext.h"
#include "Navigation/PathFollowingComponent.h"
#include "AI/Component/AO_CeilingMoveComponent.h"

EStateTreeRunStatus FAO_STTask_Chase::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FAO_STTask_Chase_InstanceData& InstanceData = Context.GetInstanceData<FAO_STTask_Chase_InstanceData>(*this);
	InstanceData.PathUpdateTimer = 0.f;
	InstanceData.bIsMoving = false;

	AAO_AggressiveAICtrl* Controller = GetAggressiveController(Context);
	if (!Controller)
	{
		return EStateTreeRunStatus::Failed;
	}

	AAO_PlayerCharacter* Target = Controller->GetChaseTarget();
	if (!Target)
	{
		return EStateTreeRunStatus::Failed;
	}

	// 추격 시작
	if (StartChasing(Controller, Target, InstanceData.AcceptanceRadius))
	{
		InstanceData.bIsMoving = true;
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FAO_STTask_Chase::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FAO_STTask_Chase_InstanceData& InstanceData = Context.GetInstanceData<FAO_STTask_Chase_InstanceData>(*this);

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

	AAO_PlayerCharacter* Target = Controller->GetChaseTarget();
	if (!Target)
	{
		// 대상이 없으면 실패
		return EStateTreeRunStatus::Failed;
	}

	// 공격 범위에 도달했으면 성공
	if (AI->IsTargetInAttackRange())
	{
		return EStateTreeRunStatus::Succeeded;
	}

	// 경로 갱신 타이머
	InstanceData.PathUpdateTimer += DeltaTime;
	if (InstanceData.PathUpdateTimer >= InstanceData.PathUpdateInterval)
	{
		InstanceData.PathUpdateTimer = 0.f;
		StartChasing(Controller, Target, InstanceData.AcceptanceRadius);
	}

	// 더 가까운 플레이어로 대상 갱신 (Controller에서 처리)
	Controller->UpdateChaseTargetToNearest();

	// KSJ:
	// Stalker 요구사항: 추적/은신 접근(Engage) 로직에서는 "천장 상태이면 안된다".
	// 천장 이동은 배회(Roam)에서만 수행하며, 전환은 전용 StateTree 상태에서만 수행한다.
	// 따라서 Chase 태스크에서의 천장 토글 로직은 제거한다.

	return EStateTreeRunStatus::Running;
}

void FAO_STTask_Chase::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FAO_STTask_Chase_InstanceData& InstanceData = Context.GetInstanceData<FAO_STTask_Chase_InstanceData>(*this);

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

AAO_AggressiveAICtrl* FAO_STTask_Chase::GetAggressiveController(FStateTreeExecutionContext& Context) const
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

bool FAO_STTask_Chase::StartChasing(AAO_AggressiveAICtrl* Controller, AAO_PlayerCharacter* Target, float AcceptRadius) const
{
	if (!Controller || !Target)
	{
		return false;
	}

	const FVector TargetLocation = Target->GetActorLocation();
	
	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalLocation(TargetLocation);
	MoveRequest.SetAcceptanceRadius(AcceptRadius);
	MoveRequest.SetUsePathfinding(true);

	const FPathFollowingRequestResult Result = Controller->MoveTo(MoveRequest);
	return Result.Code != EPathFollowingRequestResult::Failed;
}

