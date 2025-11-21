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
	UE_LOG(LogTemp, Warning, TEXT("🔥 AddEnergy Ability Activate (Pending=%f)"), PendingAmount);

	// 서버 전용 커밋 (보통 Commit은 서버에서)
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
		UE_LOG(LogTemp, Error, TEXT("AddEnergyEffectClass not assigned"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Make spec from BP GE (BP에서 Modifier를 SetByCaller로 설정해 둔다)
	FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(AddEnergyEffectClass, 1.f, ASC->MakeEffectContext());
	if (Spec.IsValid() && Spec.Data.IsValid())
	{
		FGameplayTag AmountTag = FGameplayTag::RequestGameplayTag(FName("Data.EnergyAmount"));
		Spec.Data->SetSetByCallerMagnitude(AmountTag, PendingAmount);

		float read = Spec.Data->GetSetByCallerMagnitude(AmountTag);
		UE_LOG(LogTemp, Warning, TEXT("Spec SetByCaller readback = %f"), read);

		ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());

		UE_LOG(LogTemp, Warning, TEXT("✅ Ability applied GE with %f"), PendingAmount);
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}
