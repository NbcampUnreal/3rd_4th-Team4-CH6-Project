// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/GAS/Ability/AO_GA_Werewolf_Attack.h"
#include "AI/Character/AO_Werewolf.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

UAO_GA_Werewolf_Attack::UAO_GA_Werewolf_Attack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UAO_GA_Werewolf_Attack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 몽타주 재생
	if (AttackMontage)
	{
		UAbilityTask_PlayMontageAndWait* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			TEXT("Attack"),
			AttackMontage
		);

		Task->OnBlendOut.AddDynamic(this, &UAO_GA_Werewolf_Attack::OnMontageCompleted);
		Task->OnCompleted.AddDynamic(this, &UAO_GA_Werewolf_Attack::OnMontageCompleted);
		Task->OnInterrupted.AddDynamic(this, &UAO_GA_Werewolf_Attack::OnMontageCompleted);
		Task->OnCancelled.AddDynamic(this, &UAO_GA_Werewolf_Attack::OnMontageCompleted);

		Task->ReadyForActivation();
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UAO_GA_Werewolf_Attack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UAO_GA_Werewolf_Attack::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
