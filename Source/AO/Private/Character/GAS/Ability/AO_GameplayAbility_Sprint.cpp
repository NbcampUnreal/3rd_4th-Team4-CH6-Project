// AO_GameplayAbility_Sprint.cpp

#include "Character/GAS/Ability/AO_GameplayAbility_Sprint.h"

#include "AbilitySystemComponent.h"
#include "AO_Log.h"
#include "Character/AO_PlayerCharacter.h"
#include "Character/GAS/AO_PlayerCharacter_AttributeSet.h"

UAO_GameplayAbility_Sprint::UAO_GameplayAbility_Sprint()
{
	const FGameplayTagContainer SprintTag(FGameplayTag::RequestGameplayTag(FName("Ability.Movement.Sprint")));
	SetAssetTags(SprintTag);

	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UAO_GameplayAbility_Sprint::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                 const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                 const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		AO_LOG(LogKH, Warning, TEXT("Failed to Commit Ability"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	TObjectPtr<AAO_PlayerCharacter> Character = Cast<AAO_PlayerCharacter>(ActorInfo->AvatarActor.Get());
	if (!Character)
	{
		AO_LOG(LogKH, Error, TEXT("Failed to cast AvatarActor to AAO_PlayerCharacter"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const UAO_PlayerCharacter_AttributeSet* AttributeSet = Character->GetAbilitySystemComponent()->GetSet<UAO_PlayerCharacter_AttributeSet>();
	if (!AttributeSet)
	{
		AO_LOG(LogKH, Error, TEXT("Failed to get AttributeSet"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const float CurrentStamina = AttributeSet->GetStamina();
	const float MaxStamina = AttributeSet->GetMaxStamina();

	if (bIsStaminaLockOut && CurrentStamina < MaxStamina * RequiredStaminaPercent)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (bIsStaminaLockOut && CurrentStamina >= MaxStamina * RequiredStaminaPercent)
	{
		bIsStaminaLockOut = false;
	}
	
	Character->StartSprint_GAS(true);

	Character->GetAbilitySystemComponent()->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetStaminaAttribute())
		.AddUObject(this, &UAO_GameplayAbility_Sprint::OnStaminaChanged);
	
}

void UAO_GameplayAbility_Sprint::InputReleased(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UAO_GameplayAbility_Sprint::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                            const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                            bool bReplicateEndAbility, bool bWasCancelled)
{
	if (!IsActive())
	{
		return;
	}

	if (TObjectPtr<AAO_PlayerCharacter> Character = Cast<AAO_PlayerCharacter>(ActorInfo->AvatarActor.Get()))
	{
		Character->StartSprint_GAS(false);

		if (const UAO_PlayerCharacter_AttributeSet* AttributeSet = Character->GetAbilitySystemComponent()->GetSet<UAO_PlayerCharacter_AttributeSet>())
		{
			Character->GetAbilitySystemComponent()->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetStaminaAttribute())
				.RemoveAll(this);
		}
	}

	FGameplayTagContainer SprintCostTag(FGameplayTag::RequestGameplayTag(FName("Effect.Cost.Sprint")));
	ActorInfo->AbilitySystemComponent->RemoveActiveEffectsWithTags(SprintCostTag);
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UAO_GameplayAbility_Sprint::OnStaminaChanged(const FOnAttributeChangeData& Data)
{
	if (Data.NewValue <= 0.f)
	{
		bIsStaminaLockOut = true;
		
		K2_EndAbility();
	}
}
