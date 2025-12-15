// AO_GA_AI_Stun.cpp

#include "AI/GAS/Ability/AO_GA_AI_Stun.h"
#include "AI/Base/AO_AICharacterBase.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AO_Log.h"

UAO_GA_AI_Stun::UAO_GA_AI_Stun()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	// Event.AI.Stunned 이벤트로 트리거
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = FGameplayTag::RequestGameplayTag(FName("Event.AI.Stunned"));
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);

	// Ability 태그 설정
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.State.Stunned")));

	// 기절 중에는 다른 기절 불가
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Status.Debuff.Stunned")));
}

void UAO_GA_AI_Stun::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// Ability 커밋 실패 시 종료
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// ActorInfo가 유효해야 함
	if (!ensure(ActorInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// AI 캐릭터 가져오기 - 기절 대상
	AAO_AICharacterBase* AICharacter = Cast<AAO_AICharacterBase>(ActorInfo->AvatarActor.Get());
	if (!AICharacter)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 기절 시작 알림
	AICharacter->OnStunBegin();

	// 서버에서만 이동 비활성화 및 Effect 적용
	if (HasAuthority(&ActivationInfo))
	{
		// 이동 비활성화 - 기절 중 이동 불가
		UCharacterMovementComponent* Movement = AICharacter->GetCharacterMovement();
		if (Movement)
		{
			Movement->DisableMovement();
		}

		// 기절 Effect 적용 - Status.Debuff.Stunned 태그 부여
		if (StunEffectClass)
		{
			UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
			if (ASC)
			{
				FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(Handle, ActorInfo, ActivationInfo, StunEffectClass, 1.f);
				if (SpecHandle.IsValid())
				{
					StunEffectHandle = ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
				}
			}
		}
	}

	// 기절 모션 재생
	if (StunMontage)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask = 
			UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, StunMontage);

		if (MontageTask)
		{
			MontageTask->OnCompleted.AddDynamic(this, &UAO_GA_AI_Stun::OnMontageCompleted);
			MontageTask->OnBlendOut.AddDynamic(this, &UAO_GA_AI_Stun::OnMontageCompleted);
			MontageTask->OnCancelled.AddDynamic(this, &UAO_GA_AI_Stun::OnMontageCancelled);
			MontageTask->OnInterrupted.AddDynamic(this, &UAO_GA_AI_Stun::OnMontageCancelled);

			MontageTask->ReadyForActivation();
		}
	}
	else
	{
		// 모션이 없으면 타이머로 기절 종료 - DefaultStunDuration 후 자동 해제
		UWorld* World = GetWorld();
		if (ensure(World))
		{
			FTimerHandle TimerHandle;
			World->GetTimerManager().SetTimer(
				TimerHandle,
				FTimerDelegate::CreateWeakLambda(this, [this, Handle, ActorInfo, ActivationInfo]()
				{
					EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
				}),
				DefaultStunDuration,
				false
			);
		}
	}

	AO_LOG(LogKSJ, Log, TEXT("AI Stun Ability Activated: %s"), *AICharacter->GetName());
}

void UAO_GA_AI_Stun::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	// ActorInfo 유효성 검사
	if (!ActorInfo)
	{
		Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
		return;
	}

	AAO_AICharacterBase* AICharacter = Cast<AAO_AICharacterBase>(ActorInfo->AvatarActor.Get());

	// 서버에서만 이동 복구 및 Effect 제거
	if (HasAuthority(&ActivationInfo))
	{
		// 이동 복구 - 기절 해제 후 다시 이동 가능
		if (AICharacter)
		{
			UCharacterMovementComponent* Movement = AICharacter->GetCharacterMovement();
			if (Movement)
			{
				Movement->SetMovementMode(MOVE_Walking);
			}
		}

		// 기절 Effect 제거 - Stunned 태그 해제
		if (StunEffectHandle.IsValid())
		{
			UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
			if (ASC)
			{
				ASC->RemoveActiveGameplayEffect(StunEffectHandle);
			}
			StunEffectHandle.Invalidate();
		}
	}

	// 기절 종료 알림 - 자식 클래스에서 추가 처리 가능
	if (AICharacter)
	{
		AICharacter->OnStunEnd();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	AO_LOG(LogKSJ, Log, TEXT("AI Stun Ability Ended"));
}

void UAO_GA_AI_Stun::OnMontageCompleted()
{
	if (CurrentActorInfo)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UAO_GA_AI_Stun::OnMontageCancelled()
{
	if (CurrentActorInfo)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}
}
