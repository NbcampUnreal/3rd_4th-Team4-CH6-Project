// AO_GameplayAbility_Death.cpp

#include "Character/GAS/Ability/AO_GameplayAbility_Death.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Character/AO_PlayerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

UAO_GameplayAbility_Death::UAO_GameplayAbility_Death()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
	
	const FGameplayTagContainer TraversalTag(FGameplayTag::RequestGameplayTag(FName("Ability.State.Death")));
	SetAssetTags(TraversalTag);

	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Status.Death")));
}

void UAO_GameplayAbility_Death::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (HasAuthority(&ActivationInfo))
	{
		// 피격 리액션 Ability 취소
		if (TObjectPtr<UAbilitySystemComponent> ASC = ActorInfo->AbilitySystemComponent.Get())
		{
			FGameplayTagContainer HitReactTags;
			HitReactTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.State.HitReact")));

			ASC->CancelAbilities(&HitReactTags);
		}

		TObjectPtr<ACharacter> Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
		checkf(Character, TEXT("Failed to cast AvatarActor to ACharacter"));

		Character->GetCharacterMovement()->DisableMovement();
		Character->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		if (BlockAbilitiesEffectClass)
		{
			FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(Handle, ActorInfo, ActivationInfo, BlockAbilitiesEffectClass, 1.f);
			FActiveGameplayEffectHandle BlockAbilitiesEffectHandle
				= ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
		}
	}
	
	checkf(DeathMontage, TEXT("DeathMontage is null"));

	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask
		= UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, DeathMontage);

	checkf(MontageTask, TEXT("Failed to create MontageTask"));

	MontageTask->OnCompleted.AddDynamic(this, &UAO_GameplayAbility_Death::OnMontageCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &UAO_GameplayAbility_Death::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UAO_GameplayAbility_Death::OnMontageCancelled);
	MontageTask->OnInterrupted.AddDynamic(this, &UAO_GameplayAbility_Death::OnMontageCancelled);

	MontageTask->ReadyForActivation();
}

void UAO_GameplayAbility_Death::OnMontageCompleted()
{
}

void UAO_GameplayAbility_Death::OnMontageCancelled()
{
}
