//KSJ : AO_STTask_Wolf_Attack


#include "AI/StateTree/Task/AO_STTask_Wolf_Attack.h"
#include "AI/Base/AO_AggressiveAIBase.h"
#include "AI/Controller/AO_AggressiveAICtrl.h"
#include "StateTreeExecutionContext.h"
#include "Character/AO_PlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"

EStateTreeRunStatus FAO_STTask_Wolf_Attack::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	
	AActor* OwnerActor = Cast<AActor>(Context.GetOwner());
	AAO_AggressiveAICtrl* Ctrl = nullptr;

	if (APawn* Pawn = Cast<APawn>(OwnerActor))
	{
		Ctrl = Cast<AAO_AggressiveAICtrl>(Pawn->GetController());
	}
	else if (AController* Controller = Cast<AController>(OwnerActor))
	{
		Ctrl = Cast<AAO_AggressiveAICtrl>(Controller);
	}

	InstanceData.Controller = Ctrl;

	if (!InstanceData.Controller)
	{
		return EStateTreeRunStatus::Failed;
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FAO_STTask_Wolf_Attack::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	AAO_AggressiveAICtrl* Controller = InstanceData.Controller;

	if (!Controller)
	{
		return EStateTreeRunStatus::Failed;
	}

	AAO_AggressiveAIBase* AI = Controller->GetAggressiveAI();
	AAO_PlayerCharacter* Target = Controller->GetChaseTarget();

	if (!AI || !Target)
	{
		return EStateTreeRunStatus::Failed;
	}

	// 기절 체크
	if (AI->IsStunned())
	{
		return EStateTreeRunStatus::Failed;
	}

	// 공격 범위 안에 있으면 공격
	if (AI->IsTargetInAttackRange())
	{
		Controller->StopMovement();
		
		// Ability 활성화
		UAbilitySystemComponent* ASC = AI->GetAbilitySystemComponent();
		if (ASC)
		{
			FGameplayTagContainer TagContainer;
			TagContainer.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Combat.Attack")));
			if (ASC->TryActivateAbilitiesByTag(TagContainer, true))
			{
			}
			else
			{
			}
		}
	}
	else
	{
		// 범위 밖이면 추격
		// KSJ: 공격 사거리의 50%까지 확실하게 접근하도록 수정 (기존 0.8f는 캡슐 크기 때문에 공격 사거리에 못 미쳐서 멈추는 문제 발생)
		// AcceptanceRadius가 너무 크면 이동 완료(멈춤) 판정이 공격 가능 거리(AttackRange)보다 멀리서 나서 멍하니 서있는 현상 해결
		Controller->MoveToActor(Target, AI->GetAttackRange() * 0.5f);
	}

	return EStateTreeRunStatus::Running;
}
