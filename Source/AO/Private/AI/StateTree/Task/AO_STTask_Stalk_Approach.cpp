//KSJ : AO_STTask_Stalk_Approach

#include "AI/StateTree/Task/AO_STTask_Stalk_Approach.h"
#include "AI/Controller/AO_StalkerController.h"
#include "AI/Character/AO_Stalker.h"
#include "Character/AO_PlayerCharacter.h"
#include "StateTreeExecutionContext.h"
#include "Navigation/PathFollowingComponent.h"

EStateTreeRunStatus FAO_STTask_Stalk_Approach::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	
	AActor* Owner = Cast<AActor>(Context.GetOwner());
	if (APawn* Pawn = Cast<APawn>(Owner))
	{
		InstanceData.Controller = Cast<AAO_StalkerController>(Pawn->GetController());
	}
	else if (AController* Ctrl = Cast<AController>(Owner))
	{
		InstanceData.Controller = Cast<AAO_StalkerController>(Ctrl);
	}

	if (!InstanceData.Controller)
	{
		return EStateTreeRunStatus::Failed;
	}

	AAO_PlayerCharacter* Target = InstanceData.Controller->GetChaseTarget();
	if (!Target)
	{
		return EStateTreeRunStatus::Failed;
	}

	// 플레이어가 이미 나를 보고 있으면 바로 실패 (Hide로 전환)
	if (InstanceData.Controller->IsPlayerLookingAtMe(Target, InstanceData.LookToleranceDegrees))
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.LookCheckTimer = 0.f;
	InstanceData.bIsMoving = false;

	// 타겟 위치로 이동 시작 (바닥 이동만 사용)
	InstanceData.Controller->MoveToActor(Target, InstanceData.AttackRange * 0.5f);
	InstanceData.bIsMoving = true;

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FAO_STTask_Stalk_Approach::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	if (!InstanceData.Controller)
	{
		return EStateTreeRunStatus::Failed;
	}

	AAO_Stalker* Stalker = InstanceData.Controller->GetStalker();
	AAO_PlayerCharacter* Target = InstanceData.Controller->GetChaseTarget();

	if (!Stalker || !Target)
	{
		return EStateTreeRunStatus::Failed;
	}

	// 시선 체크 (주기적으로)
	InstanceData.LookCheckTimer += DeltaTime;
	if (InstanceData.LookCheckTimer >= InstanceData.LookCheckInterval)
	{
		InstanceData.LookCheckTimer = 0.f;

		// 플레이어가 나를 보고 있으면 실패 (Hide로 전환)
		if (InstanceData.Controller->IsPlayerLookingAtMe(Target, InstanceData.LookToleranceDegrees))
		{
			return EStateTreeRunStatus::Failed;
		}
	}

	// 거리 체크
	const float DistanceToTarget = FVector::Dist(Stalker->GetActorLocation(), Target->GetActorLocation());
	
	// 공격 거리 도달 시 성공 (Attack으로 전환)
	if (DistanceToTarget <= InstanceData.AttackRange)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	// 이동 상태 확인 및 재이동
	EPathFollowingStatus::Type MoveStatus = InstanceData.Controller->GetMoveStatus();
	if (MoveStatus == EPathFollowingStatus::Idle && InstanceData.bIsMoving)
	{
		// 이동이 완료되었지만 아직 공격 거리가 아니면 다시 이동
		InstanceData.Controller->MoveToActor(Target, InstanceData.AttackRange * 0.5f);
	}

	return EStateTreeRunStatus::Running;
}

void FAO_STTask_Stalk_Approach::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	if (InstanceData.Controller)
	{
		InstanceData.Controller->StopMovement();
	}
}

