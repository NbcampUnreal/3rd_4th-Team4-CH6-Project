// AO_GameplayAbility_HitReact.cpp


#include "Character/Combat/Ability/AO_GameplayAbility_HitReact.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UAO_GameplayAbility_HitReact::UAO_GameplayAbility_HitReact()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = FGameplayTag::RequestGameplayTag(FName("Event.Combat.HitReact"));
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);

	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.State.HitReact")));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Status.Death")));
}

void UAO_GameplayAbility_HitReact::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	checkf(DefaultHitReactMontage, TEXT("DefaultHitReactMontage is null"));

	// 베이스 이벤트 태그 (Event.Combat.HitReact.Heavy)
	FGameplayTag BaseTag;
	if (TriggerEventData)
	{
		BaseTag = TriggerEventData->EventTag;
	}

	const FString DirectionSuffix = GetHitDirectionSuffix(TriggerEventData, ActorInfo);

	// 베이스 태그에 .Front / .Back 등의 접미사를 붙여서 방향까지 포함한 태그 생성
	FGameplayTag FinalTag = BaseTag;
	if (BaseTag.IsValid())
	{
		const FString BaseTagString = BaseTag.GetTagName().ToString();
		const FString FinalTagString = FString::Printf(TEXT("%s.%s"), *BaseTagString, *DirectionSuffix);
		FinalTag = FGameplayTag::RequestGameplayTag(FName(*FinalTagString));
	}

	TObjectPtr<UAnimMontage> MontageToPlay = nullptr;
	
	// 최종 태그로 몽타주 맵에서 찾아보기
	if (FinalTag.IsValid())
	{
		if (TObjectPtr<UAnimMontage>* FoundMontage = HitReactMontageMap.Find(FinalTag))
		{
			MontageToPlay = FoundMontage->Get();
		}
	}

	// 방향까지 붙힌 태그로 못 찾는 경우, 베이스 태그로도 찾아보기
	if (!MontageToPlay && BaseTag.IsValid())
	{
		if (TObjectPtr<UAnimMontage>* FoundMontage = HitReactMontageMap.Find(BaseTag))
		{
			MontageToPlay = FoundMontage->Get();
		}
	}
	
	if (!MontageToPlay)
	{
		MontageToPlay = DefaultHitReactMontage;
	}
	
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask
		= UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, MontageToPlay);
	checkf(MontageTask, TEXT("Failed to create MontageTask"));
	
	MontageTask->OnCompleted.AddDynamic(this, &UAO_GameplayAbility_HitReact::OnMontageCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &UAO_GameplayAbility_HitReact::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UAO_GameplayAbility_HitReact::OnMontageCancelled);
	MontageTask->OnInterrupted.AddDynamic(this, &UAO_GameplayAbility_HitReact::OnMontageCancelled);
	
	MontageTask->ReadyForActivation();
	
	if (HasAuthority(&ActivationInfo))
	{
		if (TObjectPtr<ACharacter> Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get()))
		{
			Character->GetCharacterMovement()->DisableMovement();
		}
		
		if (InvulnerableEffectClass)
		{
			FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(Handle, ActorInfo, ActivationInfo, InvulnerableEffectClass, 1.f);
			InvulnerableEffectHandle = ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
		}

		if (BlockAbilitiesEffectClass)
		{
			FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(Handle, ActorInfo, ActivationInfo, BlockAbilitiesEffectClass, 1.f);
			BlockAbilitiesEffectHandle = ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
		}
	}
}

void UAO_GameplayAbility_HitReact::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (HasAuthority(&ActivationInfo))
	{
		if (TObjectPtr<ACharacter> Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get()))
		{
			Character->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		}
		
		if (TObjectPtr<UAbilitySystemComponent> ASC = ActorInfo->AbilitySystemComponent.Get())
		{
			if (InvulnerableEffectHandle.IsValid())
			{
				ASC->RemoveActiveGameplayEffect(InvulnerableEffectHandle);
			}
			InvulnerableEffectHandle.Invalidate();

			if (BlockAbilitiesEffectHandle.IsValid())
			{
				ASC->RemoveActiveGameplayEffect(BlockAbilitiesEffectHandle);
			}
			BlockAbilitiesEffectHandle.Invalidate();
		}
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UAO_GameplayAbility_HitReact::OnMontageCompleted()
{
	if (CurrentActorInfo)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UAO_GameplayAbility_HitReact::OnMontageCancelled()
{
	if (CurrentActorInfo)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}
}

FString UAO_GameplayAbility_HitReact::GetHitDirectionSuffix(const FGameplayEventData* TriggerEventData,
	const FGameplayAbilityActorInfo* ActorInfo) const
{
	const TObjectPtr<AActor> InstigatorActor = TriggerEventData ? const_cast<AActor*>(TriggerEventData->Instigator.Get()) : nullptr;
	const TObjectPtr<AActor> TargetActor = ActorInfo && ActorInfo->AvatarActor.IsValid() ? ActorInfo->AvatarActor.Get() : nullptr;

	if (!InstigatorActor || !TargetActor)
	{
		return TEXT("Front");
	}

	const FVector TargetLocation = TargetActor->GetActorLocation();
	const FVector InstigatorLocation = InstigatorActor->GetActorLocation();

	FVector ToInstigator = InstigatorLocation - TargetLocation;
	ToInstigator.Z = 0.f;
	if (!ToInstigator.Normalize())
	{
		return TEXT("Front");
	}

	FVector Forward = TargetActor->GetActorForwardVector();
	Forward.Z = 0.f;
	if (!Forward.Normalize())
	{
		return TEXT("Front");
	}

	const FVector Right = FVector::CrossProduct(FVector::UpVector, Forward);

	const float ForwardDot = FVector::DotProduct(Forward, ToInstigator);
	const float RightDot = FVector::DotProduct(Right, ToInstigator);

	const float FrontThreshold = 0.707f;

	if (ForwardDot > FrontThreshold)
	{
		return TEXT("Front");
	}
	if (ForwardDot < -FrontThreshold)
	{
		return TEXT("Back");
	}
	if (RightDot >= 0.f)
	{
		return TEXT("Right");
	}
	return TEXT("Left");
}
