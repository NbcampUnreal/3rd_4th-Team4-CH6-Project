// AO_GA_Stalker_Attack.cpp

#include "AI/GAS/Ability/AO_GA_Stalker_Attack.h"
#include "AI/Character/AO_Stalker.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

UAO_GA_Stalker_Attack::UAO_GA_Stalker_Attack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UAO_GA_Stalker_Attack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AAO_Stalker* Stalker = Cast<AAO_Stalker>(ActorInfo->AvatarActor.Get());
	if (!Stalker)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 기습 공격 몽타주
	if (AttackMontage)
	{
		UAbilityTask_PlayMontageAndWait* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			TEXT("Ambush"),
			AttackMontage
		);

		Task->OnBlendOut.AddDynamic(this, &UAO_GA_Stalker_Attack::OnMontageCompleted);
		Task->OnCompleted.AddDynamic(this, &UAO_GA_Stalker_Attack::OnMontageCompleted);
		Task->OnInterrupted.AddDynamic(this, &UAO_GA_Stalker_Attack::OnMontageCompleted);
		Task->OnCancelled.AddDynamic(this, &UAO_GA_Stalker_Attack::OnMontageCompleted);

		Task->ReadyForActivation();
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UAO_GA_Stalker_Attack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 공격이 끝나면 후퇴 모드로 전환
	if (AAO_Stalker* Stalker = Cast<AAO_Stalker>(ActorInfo->AvatarActor.Get()))
	{
		Stalker->SetRetreatMode(true);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UAO_GA_Stalker_Attack::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

