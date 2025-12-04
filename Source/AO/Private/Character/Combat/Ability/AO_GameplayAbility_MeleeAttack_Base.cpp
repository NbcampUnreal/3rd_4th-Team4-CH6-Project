// AO_GameplayAbility_MeleeAttack_Base.cpp

#include "Character/Combat/Ability/AO_GameplayAbility_MeleeAttack_Base.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

#include "AO_Log.h"

UAO_GameplayAbility_MeleeAttack_Base::UAO_GameplayAbility_MeleeAttack_Base()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UAO_GameplayAbility_MeleeAttack_Base::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		AO_LOG(LogKH, Warning, TEXT("Failed to Commit Ability"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!AttackMontage || !ActorInfo->AvatarActor.IsValid())
	{
		AO_LOG(LogKH, Warning, TEXT("AttackMontage or AvatarActor is null"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask
		= UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, AttackMontage);

	if (!MontageTask)
	{
		AO_LOG(LogKH, Warning, TEXT("Failed to create MontageTask"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	MontageTask->OnCompleted.AddDynamic(this, &UAO_GameplayAbility_MeleeAttack_Base::OnMontageCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &UAO_GameplayAbility_MeleeAttack_Base::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UAO_GameplayAbility_MeleeAttack_Base::OnMontageCancelled);
	MontageTask->OnInterrupted.AddDynamic(this, &UAO_GameplayAbility_MeleeAttack_Base::OnMontageCancelled);

	MontageTask->ReadyForActivation();
}

void UAO_GameplayAbility_MeleeAttack_Base::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UAO_GameplayAbility_MeleeAttack_Base::OnMontageCompleted()
{
	if (CurrentActorInfo)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UAO_GameplayAbility_MeleeAttack_Base::OnMontageCancelled()
{
	if (CurrentActorInfo)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}
}
