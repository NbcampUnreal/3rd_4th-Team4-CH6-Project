// AO_GameplayAbility_Jump.cpp

#include "Character/GAS/Ability/AO_GameplayAbility_Jump.h"

#include "AbilitySystemComponent.h"
#include "AO_Log.h"
#include "GameFramework/Character.h"
#include "Abilities/Tasks/AbilityTask_WaitMovementModeChange.h"
#include "Character/GAS/AO_PlayerCharacter_AttributeSet.h"

UAO_GameplayAbility_Jump::UAO_GameplayAbility_Jump()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	const FGameplayTagContainer TraversalTag(FGameplayTag::RequestGameplayTag(FName("Ability.Movement.Jump")));
	SetAssetTags(TraversalTag);

	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Status.Action.Jump")));
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Status.Debuff.NoStaminaChange")));

	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Status.Action.Inspecting")));
}

bool UAO_GameplayAbility_Jump::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	const UAO_PlayerCharacter_AttributeSet* AttributeSet = ActorInfo->AbilitySystemComponent->GetSet<UAO_PlayerCharacter_AttributeSet>();
	if (!AttributeSet)
	{
		return false;
	}
	
	const float CurrentStamina = AttributeSet->GetStamina();
	if (CurrentStamina < StaminaCost)
	{
		if (OptionalRelevantTags)
		{
			OptionalRelevantTags->AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Fail.NotEnoughStamina")));
		}
		return false;
	}
	
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	if (IsActive())
	{
		return false;
	}

	ACharacter* Character = CastChecked<ACharacter>(ActorInfo->AvatarActor.Get());
	if (!Character->CanJump())
	{
		return false;
	}

	return true;
}

void UAO_GameplayAbility_Jump::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                               const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                               const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ACharacter* Character = CastChecked<ACharacter>(ActorInfo->AvatarActor.Get());

	Character->Jump();

	UAbilityTask_WaitMovementModeChange* WaitTask
		= UAbilityTask_WaitMovementModeChange::CreateWaitMovementModeChange(this, MOVE_Walking);

	WaitTask->OnChange.AddDynamic(this, &UAO_GameplayAbility_Jump::OnMovementModeChanged);
	WaitTask->ReadyForActivation();
}

void UAO_GameplayAbility_Jump::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (!IsActive())
	{
		return;
	}
	
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	checkf(ASC, TEXT("Failed to get AbilitySystemComponent"));

	checkf(PostSprintNoChangeEffectClass, TEXT("PostSprintNoChangeEffectClass is null"));
	if (ActorInfo->IsNetAuthority())
	{
		const FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(PostSprintNoChangeEffectClass, 1.f, Context);

		if (SpecHandle.IsValid())
		{
			ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UAO_GameplayAbility_Jump::OnMovementModeChanged(EMovementMode NewMovementMode)
{
	if (IsActive() && NewMovementMode == MOVE_Walking)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}
