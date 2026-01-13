//KSJ : AO_STTask_Stalk_Ambush

#include "AI/StateTree/Task/AO_STTask_Stalk_Ambush.h"
#include "AI/Controller/AO_StalkerController.h"
#include "AI/Character/AO_Stalker.h"
#include "StateTreeExecutionContext.h"
#include "AbilitySystemComponent.h"

EStateTreeRunStatus FAO_STTask_Stalk_Ambush::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
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

	AAO_Stalker* Stalker = InstanceData.Controller->GetStalker();
	if (!Stalker)
	{
		return EStateTreeRunStatus::Failed;
	}

	// 기절 상태면 공격 불가
	if (Stalker->IsStunned())
	{
		return EStateTreeRunStatus::Failed;
	}

	UAbilitySystemComponent* ASC = Stalker->GetAbilitySystemComponent();
	if (!ASC)
	{
		return EStateTreeRunStatus::Failed;
	}

	// 공격 실행 (Ability.Action.Ambush 태그 사용)
	// Stalker GA는 이 태그로 트리거되도록 설정되어 있어야 함
	FGameplayTag AttackTag = FGameplayTag::RequestGameplayTag(FName("Ability.Action.Ambush"));
	if (ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(AttackTag)))
	{
		InstanceData.bIsAttacking = true;
		InstanceData.bWaitingForAttackEnd = true;
		return EStateTreeRunStatus::Running;
	}

	return EStateTreeRunStatus::Failed;
}

EStateTreeRunStatus FAO_STTask_Stalk_Ambush::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	if (!InstanceData.Controller) return EStateTreeRunStatus::Failed;

	AAO_Stalker* Stalker = InstanceData.Controller->GetStalker();
	if (!Stalker) return EStateTreeRunStatus::Failed;

	// 기절 체크
	if (Stalker->IsStunned())
	{
		InstanceData.bIsAttacking = false;
		return EStateTreeRunStatus::Failed;
	}

	// 공격 완료 대기
	if (InstanceData.bWaitingForAttackEnd)
	{
		// Stalker의 IsAttacking 상태 확인 (GA에서 관리)
		if (!Stalker->IsAttacking())
		{
			// 공격 완료
			InstanceData.bIsAttacking = false;
			InstanceData.bWaitingForAttackEnd = false;
			return EStateTreeRunStatus::Succeeded;
		}
	}

	return EStateTreeRunStatus::Running;
}
