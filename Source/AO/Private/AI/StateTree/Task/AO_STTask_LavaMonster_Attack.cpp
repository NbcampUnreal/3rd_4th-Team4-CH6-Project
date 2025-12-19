// AO_STTask_LavaMonster_Attack.cpp

#include "AI/StateTree/Task/AO_STTask_LavaMonster_Attack.h"
#include "AI/Character/AO_LavaMonster.h"
#include "AI/Controller/AO_LavaMonsterCtrl.h"
#include "StateTreeExecutionContext.h"
#include "AO_Log.h"

EStateTreeRunStatus FAO_STTask_LavaMonster_Attack::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FAO_STTask_LavaMonster_Attack_InstanceData& InstanceData = Context.GetInstanceData<FAO_STTask_LavaMonster_Attack_InstanceData>(*this);
	InstanceData.bIsAttacking = false;
	InstanceData.bWaitingForAttackEnd = false;

	AAO_LavaMonsterCtrl* Controller = GetLavaMonsterController(Context);
	AAO_LavaMonster* LavaMonster = GetLavaMonster(Context);

	if (!Controller || !LavaMonster)
	{
		return EStateTreeRunStatus::Failed;
	}

	// 공격 가능 여부 확인
	if (!Controller->CanAttack())
	{
		return EStateTreeRunStatus::Failed;
	}

	// 사거리 기반 공격 타입 선택
	const ELavaMonsterAttackType AttackType = LavaMonster->SelectAttackTypeByRange();
	LavaMonster->ExecuteAttack(AttackType);

	InstanceData.bIsAttacking = true;
	InstanceData.bWaitingForAttackEnd = true;

	AO_LOG(LogKSJ, Log, TEXT("LavaMonster attack started: type %d"), static_cast<int32>(AttackType));

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FAO_STTask_LavaMonster_Attack::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FAO_STTask_LavaMonster_Attack_InstanceData& InstanceData = Context.GetInstanceData<FAO_STTask_LavaMonster_Attack_InstanceData>(*this);

	AAO_LavaMonster* LavaMonster = GetLavaMonster(Context);
	if (!LavaMonster)
	{
		return EStateTreeRunStatus::Failed;
	}

	// 기절 상태면 중단
	if (LavaMonster->IsStunned())
	{
		InstanceData.bIsAttacking = false;
		return EStateTreeRunStatus::Failed;
	}

	// 공격 완료 대기
	if (InstanceData.bWaitingForAttackEnd)
	{
		if (!LavaMonster->IsAttacking())
		{
			// 공격 완료
			InstanceData.bIsAttacking = false;
			InstanceData.bWaitingForAttackEnd = false;
			return EStateTreeRunStatus::Succeeded;
		}
	}

	return EStateTreeRunStatus::Running;
}

void FAO_STTask_LavaMonster_Attack::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FAO_STTask_LavaMonster_Attack_InstanceData& InstanceData = Context.GetInstanceData<FAO_STTask_LavaMonster_Attack_InstanceData>(*this);
	InstanceData.bIsAttacking = false;
	InstanceData.bWaitingForAttackEnd = false;
}

AAO_LavaMonsterCtrl* FAO_STTask_LavaMonster_Attack::GetLavaMonsterController(FStateTreeExecutionContext& Context) const
{
	if (AActor* Owner = Cast<AActor>(Context.GetOwner()))
	{
		if (AAO_LavaMonsterCtrl* Controller = Cast<AAO_LavaMonsterCtrl>(Owner))
		{
			return Controller;
		}
		if (APawn* Pawn = Cast<APawn>(Owner))
		{
			return Cast<AAO_LavaMonsterCtrl>(Pawn->GetController());
		}
	}
	return nullptr;
}

AAO_LavaMonster* FAO_STTask_LavaMonster_Attack::GetLavaMonster(FStateTreeExecutionContext& Context) const
{
	AAO_LavaMonsterCtrl* Controller = GetLavaMonsterController(Context);
	if (Controller)
	{
		return Controller->GetLavaMonster();
	}
	return nullptr;
}

