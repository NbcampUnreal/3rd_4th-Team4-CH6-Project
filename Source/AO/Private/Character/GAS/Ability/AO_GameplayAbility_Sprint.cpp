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
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Status.Action.Sprint")));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Status.Lockout.Stamina")));
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

	const UAO_PlayerCharacter_AttributeSet* AttributeSet = ActorInfo->AbilitySystemComponent->GetSetChecked<UAO_PlayerCharacter_AttributeSet>();
	const float CurrentStamina = AttributeSet->GetStamina();
	const float MaxStamina = AttributeSet->GetMaxStamina();

	if (CurrentStamina < MaxStamina * AttributeSet->StaminaLockoutPercent)
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
	
	Character->StartSprint_GAS(true);
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
	}

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	checkf(ASC, TEXT("Failed to get AbilitySystemComponent"));
	
	const FGameplayTagContainer SprintCostTag(FGameplayTag::RequestGameplayTag(FName("Effect.Cost.Sprint")));
	ASC->RemoveActiveEffectsWithTags(SprintCostTag);

	checkf(PostSprintNoRegenEffectClass, TEXT("PostSprintNoRegenEffectClass is null"));
	if (ActorInfo->IsNetAuthority())
	{
		const FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(PostSprintNoRegenEffectClass, 1.f, Context);

		if (SpecHandle.IsValid())
		{
			ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}