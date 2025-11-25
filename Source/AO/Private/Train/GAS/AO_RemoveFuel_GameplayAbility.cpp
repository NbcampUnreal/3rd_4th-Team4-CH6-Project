#include "Train/GAS/AO_RemoveFuel_GameplayAbility.h"
#include "AbilitySystemComponent.h"

UAO_RemoveFuel_GameplayAbility::UAO_RemoveFuel_GameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	PendingAmount = -5.f;
}

void UAO_RemoveFuel_GameplayAbility::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!ActorInfo || !ActorInfo->IsNetAuthority())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!DeleteEnergyEffectClass || !ActorInfo->AbilitySystemComponent.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	FGameplayEffectContextHandle EffectContext = ActorInfo->AbilitySystemComponent->MakeEffectContext();
	FGameplayEffectSpecHandle SpecHandle = ActorInfo->AbilitySystemComponent->MakeOutgoingSpec(DeleteEnergyEffectClass, 1.f, EffectContext);

	if (SpecHandle.IsValid() && SpecHandle.Data.IsValid())
	{
		FGameplayTag AmountTag = FGameplayTag::RequestGameplayTag(FName("Data.EnergyAmount"));
		SpecHandle.Data->SetSetByCallerMagnitude(AmountTag, PendingAmount);
		
		ActiveGEHandle = ActorInfo->AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
	}

	UE_LOG(LogTemp, Warning, TEXT("EnergyLeak Activated, GE applied"));
}

void UAO_RemoveFuel_GameplayAbility::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid() && ActiveGEHandle.IsValid())
	{
		ActorInfo->AbilitySystemComponent->RemoveActiveGameplayEffect(ActiveGEHandle);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
