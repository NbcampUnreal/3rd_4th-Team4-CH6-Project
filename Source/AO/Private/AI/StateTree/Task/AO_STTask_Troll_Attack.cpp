// AO_STTask_Troll_Attack.cpp

#include "AI/StateTree/Task/AO_STTask_Troll_Attack.h"
#include "AI/Character/AO_Troll.h"
#include "AI/Controller/AO_TrollController.h"
#include "StateTreeExecutionContext.h"
#include "AO_Log.h"

EStateTreeRunStatus FAO_STTask_Troll_Attack::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FAO_STTask_Troll_Attack_InstanceData& InstanceData = Context.GetInstanceData<FAO_STTask_Troll_Attack_InstanceData>(*this);
	InstanceData.bIsAttacking = false;
	InstanceData.bWaitingForAttackEnd = false;

	AAO_TrollController* Controller = GetTrollController(Context);
	AAO_Troll* Troll = GetTroll(Context);

	if (!Controller || !Troll)
	{
		return EStateTreeRunStatus::Failed;
	}

	// 공격 가능 여부 확인
	if (!Controller->CanAttack())
	{
		return EStateTreeRunStatus::Failed;
	}

	// 랜덤 공격 실행
	const ETrollAttackType AttackType = Troll->SelectRandomAttackType();
	Troll->ExecuteAttack(AttackType);

	InstanceData.bIsAttacking = true;
	InstanceData.bWaitingForAttackEnd = true;

	AO_LOG(LogKSJ, Log, TEXT("Troll attack started: type %d"), static_cast<int32>(AttackType));

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FAO_STTask_Troll_Attack::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FAO_STTask_Troll_Attack_InstanceData& InstanceData = Context.GetInstanceData<FAO_STTask_Troll_Attack_InstanceData>(*this);

	AAO_Troll* Troll = GetTroll(Context);
	if (!Troll)
	{
		return EStateTreeRunStatus::Failed;
	}

	// 기절 상태면 중단
	if (Troll->IsStunned())
	{
		InstanceData.bIsAttacking = false;
		return EStateTreeRunStatus::Failed;
	}

	// 공격 완료 대기
	if (InstanceData.bWaitingForAttackEnd)
	{
		if (!Troll->IsAttacking())
		{
			// 공격 완료
			InstanceData.bIsAttacking = false;
			InstanceData.bWaitingForAttackEnd = false;
			return EStateTreeRunStatus::Succeeded;
		}
	}

	return EStateTreeRunStatus::Running;
}

void FAO_STTask_Troll_Attack::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FAO_STTask_Troll_Attack_InstanceData& InstanceData = Context.GetInstanceData<FAO_STTask_Troll_Attack_InstanceData>(*this);
	InstanceData.bIsAttacking = false;
	InstanceData.bWaitingForAttackEnd = false;
}

AAO_TrollController* FAO_STTask_Troll_Attack::GetTrollController(FStateTreeExecutionContext& Context) const
{
	if (AActor* Owner = Cast<AActor>(Context.GetOwner()))
	{
		if (AAO_TrollController* Controller = Cast<AAO_TrollController>(Owner))
		{
			return Controller;
		}
		if (APawn* Pawn = Cast<APawn>(Owner))
		{
			return Cast<AAO_TrollController>(Pawn->GetController());
		}
	}
	return nullptr;
}

AAO_Troll* FAO_STTask_Troll_Attack::GetTroll(FStateTreeExecutionContext& Context) const
{
	AAO_TrollController* Controller = GetTrollController(Context);
	if (Controller)
	{
		return Controller->GetTroll();
	}
	return nullptr;
}

