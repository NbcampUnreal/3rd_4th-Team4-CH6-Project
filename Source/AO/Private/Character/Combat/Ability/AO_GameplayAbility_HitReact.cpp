// AO_GameplayAbility_HitReact.cpp


#include "Character/Combat/Ability/AO_GameplayAbility_HitReact.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

#include "AO_Log.h"

UAO_GameplayAbility_HitReact::UAO_GameplayAbility_HitReact()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = FGameplayTag::RequestGameplayTag(FName("Event.Combat.HitReact"));
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
}

void UAO_GameplayAbility_HitReact::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		AO_LOG(LogKH, Warning, TEXT("Failed to Commit Ability"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!DefaultHitReactMontage)
	{
		AO_LOG(LogKH, Warning, TEXT("DefaultHitReactMontage is null"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	FGameplayTag EventTag;
	if (TriggerEventData)
	{
		EventTag = TriggerEventData->EventTag;
	}

	TObjectPtr<UAnimMontage> MontageToPlay = nullptr;
	if (EventTag.IsValid())
	{
		if (TObjectPtr<UAnimMontage>* FoundMontage = HitReactMontageMap.Find(EventTag))
		{
			MontageToPlay = FoundMontage->Get();
		}
	}
	
	if (!MontageToPlay)
	{
		MontageToPlay = DefaultHitReactMontage;
	}
	
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask
		= UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, MontageToPlay);
	
	if (!MontageTask)
	{
		AO_LOG(LogKH, Warning, TEXT("Failed to create MontageTask"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	MontageTask->OnCompleted.AddDynamic(this, &UAO_GameplayAbility_HitReact::OnMontageCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &UAO_GameplayAbility_HitReact::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UAO_GameplayAbility_HitReact::OnMontageCancelled);
	MontageTask->OnInterrupted.AddDynamic(this, &UAO_GameplayAbility_HitReact::OnMontageCancelled);
	
	MontageTask->ReadyForActivation();
}

void UAO_GameplayAbility_HitReact::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UAO_GameplayAbility_HitReact::OnMontageCompleted()
{
	if (CurrentActorInfo)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UAO_GameplayAbility_HitReact::OnMontageCancelled()
{
	if (CurrentActorInfo)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}
}
