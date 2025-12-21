// AO_STTask_Stalk_Ambush.cpp

#include "AI/StateTree/Task/AO_STTask_Stalk_Ambush.h"
#include "AI/Controller/AO_StalkerController.h"
#include "AI/Character/AO_Stalker.h"
#include "Character/AO_PlayerCharacter.h"
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

	InstanceData.AmbushTimer = 0.f;

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FAO_STTask_Stalk_Ambush::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	if (!InstanceData.Controller) return EStateTreeRunStatus::Failed;

	AAO_Stalker* Stalker = InstanceData.Controller->GetStalker();
	AAO_PlayerCharacter* Target = InstanceData.Controller->GetChaseTarget();

	if (!Stalker || !Target) return EStateTreeRunStatus::Failed;

	// 기습 조건 체크 (거리, 후방 등)
	float Dist = FVector::Dist(Stalker->GetActorLocation(), Target->GetActorLocation());
	
	// 공격 사거리 내 진입 시 공격 실행
	if (Dist < 200.f)
	{
		UAbilitySystemComponent* ASC = Stalker->GetAbilitySystemComponent();
		if (ASC)
		{
			FGameplayTag AttackTag = FGameplayTag::RequestGameplayTag(FName("Ability.Action.Ambush"));
			ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(AttackTag));
			return EStateTreeRunStatus::Succeeded; // 공격 실행했으므로 Task 종료 (후퇴 등 다음 상태로)
		}
	}

	// 플레이어가 보고 있으면 다시 숨기로 전환 (Task 실패로 처리하여 Selector에서 Hide로 넘어가도록)
	// Condition으로 빼는게 좋지만 여기서 간단 체크
	FVector DirToStalker = (Stalker->GetActorLocation() - Target->GetActorLocation()).GetSafeNormal();
	float Dot = FVector::DotProduct(Target->GetActorForwardVector(), DirToStalker);
	if (Dot > 0.5f) // 전방 60도 내
	{
		// 들켰다! 숨으러 가자
		return EStateTreeRunStatus::Failed; 
	}

	return EStateTreeRunStatus::Running;
}

