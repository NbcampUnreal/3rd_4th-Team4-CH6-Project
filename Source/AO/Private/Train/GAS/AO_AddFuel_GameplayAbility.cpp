#include "Train/GAS/AO_AddFuel_GameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"


UAO_AddFuel_GameplayAbility::UAO_AddFuel_GameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UAO_AddFuel_GameplayAbility::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!ActorInfo->IsNetAuthority() || !CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (!ASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	if (!AddEnergyEffectClass)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(AddEnergyEffectClass, 1.f, ASC->MakeEffectContext());
	if (Spec.IsValid() && Spec.Data.IsValid())
	{
		FGameplayTag AmountTag = FGameplayTag::RequestGameplayTag(FName("Data.EnergyAmount"));
		Spec.Data->SetSetByCallerMagnitude(AmountTag, PendingAmount);

		ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}
