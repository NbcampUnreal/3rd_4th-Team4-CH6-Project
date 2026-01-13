// AO_GameplayAbility_Death.cpp

#include "Character/GAS/Ability/AO_GameplayAbility_Death.h"

#include "AbilitySystemComponent.h"
#include "AO_Log.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/AO_PlayerCharacter.h"
#include "Character/Components/AO_DeathSpectateComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

UAO_GameplayAbility_Death::UAO_GameplayAbility_Death()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	
	const FGameplayTagContainer TraversalTag(FGameplayTag::RequestGameplayTag(FName("Ability.State.Death")));
	SetAssetTags(TraversalTag);
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

	if (!ActorInfo || !ActorInfo->IsNetAuthority())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}

	bDeathFinalizeCalled = false;

	TObjectPtr<UAbilitySystemComponent> ASC = ActorInfo->AbilitySystemComponent.Get();
	if (!ensure(ASC))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	// 이미 캐릭터가 사망한 경우 취소
	if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Status.Death"))))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 피격 리액션 Ability 취소
	FGameplayTagContainer HitReactTags;
	HitReactTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.State.HitReact")));
	
	ASC->CancelAbilities(&HitReactTags);

	// 사망 사운드 재생
	ASC->ExecuteGameplayCue(FGameplayTag::RequestGameplayTag(FName("GameplayCue.Player.Death")));

	// 래그돌 이벤트 대기
	UAbilityTask_WaitGameplayEvent* WaitRagdollEventTask
		= UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, RagdollEventTag, nullptr, true, true);

	checkf(WaitRagdollEventTask, TEXT("Failed to create WaitRagdollEventTask"));

	WaitRagdollEventTask->EventReceived.AddDynamic(this, &UAO_GameplayAbility_Death::OnRagdollEventReceived);
	WaitRagdollEventTask->ReadyForActivation();
	
	// 사망 몽타주 재생
	checkf(DeathMontage, TEXT("DeathMontage is null"));

	AAO_PlayerCharacter* PlayerCharacter = Cast<AAO_PlayerCharacter>(ActorInfo->AvatarActor.Get());
	checkf(PlayerCharacter, TEXT("Failed to cast AvatarActor to AAO_PlayerCharacter"));

	UAO_DeathSpectateComponent* DeathSpectateComponent = PlayerCharacter->FindComponentByClass<UAO_DeathSpectateComponent>();
	checkf(DeathSpectateComponent, TEXT("Failed to find DeathSpectateComponent"));

	DeathSpectateComponent->MulticastRPC_PlayDeathMontage(DeathMontage);
	
	if (ActorInfo->IsNetAuthority())
	{
		TObjectPtr<ACharacter> Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
		checkf(Character, TEXT("Failed to cast AvatarActor to ACharacter"));

		Character->GetCharacterMovement()->StopMovementImmediately();
		Character->GetCharacterMovement()->DisableMovement();

		// 사망 태그 적용
		checkf(DeathTagEffectClass, TEXT("DeathTagEffectClass is null"));
		FGameplayEffectSpecHandle DeathTagSpecHandle = MakeOutgoingGameplayEffectSpec(Handle, ActorInfo, ActivationInfo, DeathTagEffectClass, 1.f);
		FActiveGameplayEffectHandle DeathTagEffectHandle = ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, DeathTagSpecHandle);

		// 다른 Ability 차단 태그 적용
		checkf(BlockAbilitiesEffectClass, TEXT("BlockAbilitiesEffectClass is null"));
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(Handle, ActorInfo, ActivationInfo, BlockAbilitiesEffectClass, 1.f);
		FActiveGameplayEffectHandle BlockAbilitiesEffectHandle = ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
		
		const float FallbackSeconds = DeathMontage->GetPlayLength();
		const float SafeSeconds = (FallbackSeconds > 0.f) ? FallbackSeconds : 1.f;

		if (UWorld* World = ActorInfo->AvatarActor->GetWorld())
		{
			World->GetTimerManager().SetTimer(
				DeathFinalizeTimerHandle,
				this,
				&UAO_GameplayAbility_Death::OnDeathFinalizeTimerExpired,
				SafeSeconds,
				false);
		}
	}
}

void UAO_GameplayAbility_Death::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		if (UWorld* World = ActorInfo->AvatarActor->GetWorld())
		{
			World->GetTimerManager().ClearTimer(DeathFinalizeTimerHandle);
		}
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UAO_GameplayAbility_Death::CancelAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateCancelAbility)
{
	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		if (UWorld* World = ActorInfo->AvatarActor->GetWorld())
		{
			World->GetTimerManager().ClearTimer(DeathFinalizeTimerHandle);
		}
	}
	
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

void UAO_GameplayAbility_Death::OnRagdollEventReceived(FGameplayEventData Payload)
{
	if (!CurrentActorInfo->IsNetAuthority())
	{
		return;
	}
	
	FinalizeDeath(CurrentActorInfo, true);
}

void UAO_GameplayAbility_Death::FinalizeDeath(const FGameplayAbilityActorInfo* ActorInfo, bool bFromNotify)
{
	if (bDeathFinalizeCalled)
	{
		return;
	}
	bDeathFinalizeCalled = true;

	if (!bFromNotify)
	{
		AO_LOG(LogKH, Warning, TEXT("FinalizeDeath called without notify"));
	}
	
	AAO_PlayerCharacter* PlayerCharacter = Cast<AAO_PlayerCharacter>(CurrentActorInfo->AvatarActor.Get());
	if (!PlayerCharacter)
	{
		return;
	}

	if (UAbilitySystemComponent* ASC = CurrentActorInfo->AbilitySystemComponent.Get())
	{
		ASC->CurrentMontageStop(0.05f);
	}

	if (UAO_DeathSpectateComponent* DeathSpectateComponent = PlayerCharacter->FindComponentByClass<UAO_DeathSpectateComponent>())
	{
		DeathSpectateComponent->MulticastRPC_EnterRagdoll();
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UAO_GameplayAbility_Death::OnDeathFinalizeTimerExpired()
{
	if (CurrentActorInfo)
	{
		FinalizeDeath(CurrentActorInfo, false);
	}
}
