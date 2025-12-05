// AO_GameplayAbility_Sprint.cpp

#include "Character/GAS/Ability/AO_GameplayAbility_Sprint.h"

#include "AbilitySystemComponent.h"
#include "AO_Log.h"
#include "Character/AO_PlayerCharacter.h"
#include "Character/GAS/AO_PlayerCharacter_AttributeSet.h"
#include "GameFramework/CharacterMovementComponent.h"

UAO_GameplayAbility_Sprint::UAO_GameplayAbility_Sprint()
{
	const FGameplayTagContainer SprintTag(FGameplayTag::RequestGameplayTag(FName("Ability.Movement.Sprint")));
	SetAssetTags(SprintTag);

	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UAO_GameplayAbility_Sprint::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);

	Character = Cast<AAO_PlayerCharacter>(ActorInfo->AvatarActor.Get());
	checkf(Character, TEXT("Failed to cast AvatarActor to ACharacter"));

	CharacterMovement = Character->GetCharacterMovement();
	checkf(CharacterMovement, TEXT("Failed to get CharacterMovementComponent"));
}

bool UAO_GameplayAbility_Sprint::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                    const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
                                                    const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	if (CharacterMovement->Velocity.SizeSquared2D() < KINDA_SMALL_NUMBER)
	{
		return false;
	}

	if (CharacterMovement->IsCrouching())
	{
		return false;
	}

	return true;
}

void UAO_GameplayAbility_Sprint::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                 const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                 const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const UAO_PlayerCharacter_AttributeSet* AttributeSet = Character->GetAbilitySystemComponent()->GetSet<UAO_PlayerCharacter_AttributeSet>();
	checkf(AttributeSet, TEXT("Failed to get AttributeSet"));
	
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

void UAO_GameplayAbility_Sprint::InputPressed(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputPressed(Handle, ActorInfo, ActivationInfo);
	
	if (CharacterMovement->Velocity.SizeSquared2D() < KINDA_SMALL_NUMBER)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
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

	if (Character)
	{
		Character->StartSprint_GAS(false);

		if (const UAO_PlayerCharacter_AttributeSet* AttributeSet = Character->GetAbilitySystemComponent()->GetSet<UAO_PlayerCharacter_AttributeSet>())
		{
			Character->GetAbilitySystemComponent()->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetStaminaAttribute())
				.RemoveAll(this);
		}
	}

	const FGameplayTagContainer SprintCostTag(FGameplayTag::RequestGameplayTag(FName("Effect.Cost.Sprint")));
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
