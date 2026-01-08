//KSJ : AO_GA_AIAttackBase

#include "AI/GAS/Ability/AO_GA_AIAttackBase.h"
#include "AI/Base/AO_AICharacterBase.h"
#include "Character/Combat/AO_MeleeHitEventPayload.h"
#include "Character/AO_PlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"

UAO_GA_AIAttackBase::UAO_GA_AIAttackBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	KnockdownHitReactTag = FGameplayTag::RequestGameplayTag(FName("Event.Combat.HitReact.Knockdown"));
	DefaultHitReactTag = FGameplayTag::RequestGameplayTag(FName("Event.Combat.HitReact.Light"));
}

bool UAO_GA_AIAttackBase::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	AAO_AICharacterBase* AIChar = Cast<AAO_AICharacterBase>(ActorInfo->AvatarActor.Get());
	if (!AIChar)
	{
		return false;
	}

	// 기절 상태면 불가
	if (AIChar->IsStunned())
	{
		return false;
	}

	// 이미 공격 중이면 불가
	if (AIChar->IsAttacking())
	{
		return false;
	}

	return true;
}

void UAO_GA_AIAttackBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AAO_AICharacterBase* AIChar = Cast<AAO_AICharacterBase>(ActorInfo->AvatarActor.Get());
	if (!AIChar)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 1. 공격 설정 가져오기
	CurrentAttackConfig = AIChar->GetCurrentAttackConfig();

	if (!CurrentAttackConfig.AttackMontage)
	{
		// 몽타주가 없으면 즉시 종료
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 2. 공격 상태 설정
	AIChar->SetIsAttacking(true);

	// 3. 히트 이벤트 대기 Task (AnimNotify)
	UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		FGameplayTag::RequestGameplayTag(FName("Event.Combat.Confirm")),
		nullptr,
		false,
		false
	);

	if (WaitEventTask)
	{
		WaitEventTask->EventReceived.AddDynamic(this, &UAO_GA_AIAttackBase::OnHitConfirmEvent);
		WaitEventTask->ReadyForActivation();
	}

	// 4. 몽타주 재생 Task
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		CurrentAttackConfig.AttackMontage
	);

	if (MontageTask)
	{
		MontageTask->OnCompleted.AddDynamic(this, &UAO_GA_AIAttackBase::OnMontageCompleted);
		MontageTask->OnCancelled.AddDynamic(this, &UAO_GA_AIAttackBase::OnMontageCancelled);
		MontageTask->OnInterrupted.AddDynamic(this, &UAO_GA_AIAttackBase::OnMontageCancelled);
		MontageTask->ReadyForActivation();
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

void UAO_GA_AIAttackBase::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 공격 상태 해제
	if (AAO_AICharacterBase* AIChar = Cast<AAO_AICharacterBase>(ActorInfo->AvatarActor.Get()))
	{
		AIChar->SetIsAttacking(false);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UAO_GA_AIAttackBase::OnHitConfirmEvent(FGameplayEventData Payload)
{
	AAO_AICharacterBase* AIChar = Cast<AAO_AICharacterBase>(GetAvatarActorFromActorInfo());
	if (!AIChar) return;

	FVector TraceStart;
	FVector TraceEnd;
	float TraceRadius = CurrentAttackConfig.AttackRadius;

	// 1. Payload에서 Trace 정보 가져오기 시도
	const UAO_MeleeHitEventPayload* HitPayload = Cast<UAO_MeleeHitEventPayload>(Payload.OptionalObject);
	if (HitPayload)
	{
		TraceStart = HitPayload->Params.TraceStart;
		TraceEnd = HitPayload->Params.TraceEnd;
	}
	else
	{
		// 2. Fallback: Payload 없으면 정면으로 Trace
		TraceStart = AIChar->GetActorLocation();
		TraceEnd = TraceStart + (AIChar->GetActorForwardVector() * CurrentAttackConfig.AttackDistance);
	}

	// Sphere Trace 수행
	TArray<FHitResult> HitResults;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(AIChar);

	bool bHit = UKismetSystemLibrary::SphereTraceMulti(
		GetWorld(),
		TraceStart,
		TraceEnd,
		TraceRadius,
		UEngineTypes::ConvertToTraceType(TraceChannel),
		false,
		ActorsToIgnore,
		EDrawDebugTrace::None,
		HitResults,
		true,
		FLinearColor::Red,
		FLinearColor::Green,
		1.f
	);

	if (bHit)
	{
		TSet<AActor*> ProcessedActors;

		for (const FHitResult& Hit : HitResults)
		{
			AActor* HitActor = Hit.GetActor();
			if (!HitActor || ProcessedActors.Contains(HitActor)) continue;

			// 플레이어만 타격 (필요 시 조건 확장 가능)
			if (Cast<AAO_PlayerCharacter>(HitActor))
			{
				ProcessedActors.Add(HitActor);
				ApplyDamageAndKnockback(HitActor, AIChar, CurrentAttackConfig);
			}
		}
	}
}

void UAO_GA_AIAttackBase::ApplyDamageAndKnockback(AActor* TargetActor, AActor* InstigatorActor, const FEnemyAttackConfig& Config)
{
	if (!TargetActor || !InstigatorActor) return;

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);

	if (!SourceASC || !TargetASC) return;

	// 무적 확인
	if (TargetASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Status.Invulnerable"))))
	{
		return;
	}

	// 타격음 재생 (무적 체크 통과 후, 실제로 맞았을 때만)
	if (HitSound)
	{
		MulticastPlayHitSound(TargetActor->GetActorLocation());
	}

	// 1. 데미지 적용
	if (Config.DamageEffectClass)
	{
		FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
		Context.AddInstigator(InstigatorActor, InstigatorActor);

		FGameplayEffectSpecHandle DamageSpec = SourceASC->MakeOutgoingSpec(Config.DamageEffectClass, 1.f, Context);
		if (DamageSpec.IsValid())
		{
			// SetByCaller로 데미지 전달
			const FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(FName("Data.Damage"));
			DamageSpec.Data.Get()->SetSetByCallerMagnitude(DamageTag, Config.Damage);
			SourceASC->ApplyGameplayEffectSpecToTarget(*DamageSpec.Data.Get(), TargetASC);
		}
	}

	// 3. Hit React 이벤트 전송 (먼저 실행하여 피격 상태로 만듦)
	SendHitReactEvent(TargetActor, InstigatorActor, Config.Damage);

	// 2. 넉백 적용 (피격 상태로 멈춘 것을 강제로 날려버림)
	ACharacter* TargetChar = Cast<ACharacter>(TargetActor);
	if (TargetChar && Config.KnockbackStrength > 0.f)
	{
		FVector KnockbackDir = (TargetActor->GetActorLocation() - InstigatorActor->GetActorLocation()).GetSafeNormal();
		KnockbackDir.Z = 0.2f; // 약간 위로
		KnockbackDir.Normalize();

		if (UCharacterMovementComponent* CMC = TargetChar->GetCharacterMovement())
		{
			CMC->SetMovementMode(MOVE_Falling);
		}
		TargetChar->LaunchCharacter(KnockbackDir * Config.KnockbackStrength, true, true);
	}

	// 4. 자식 클래스 오버라이드 콜백
	OnTargetHit(TargetActor, InstigatorActor);
}

void UAO_GA_AIAttackBase::MulticastPlayHitSound_Implementation(const FVector& Location)
{
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			HitSound,
			Location,
			1.f,
			1.f,
			0.f,
			HitSoundAttenuation
		);
	}
}

void UAO_GA_AIAttackBase::OnTargetHit(AActor* TargetActor, AActor* InstigatorActor)
{
	// 기본 구현 없음
}

void UAO_GA_AIAttackBase::SendHitReactEvent(AActor* TargetActor, AActor* InstigatorActor, float Damage)
{
	if (!TargetActor) return;

	// 데미지 크기나 설정에 따라 태그 결정 (여기서는 넉백 강도가 높으면 Knockdown으로 간주하는 등 로직 추가 가능)
	// 단순화를 위해 Config에 따라 결정하거나 기본값 사용
	
	FGameplayTag HitTag = DefaultHitReactTag;
	if (CurrentAttackConfig.KnockbackStrength >= 400.f) // 임시 기준: 넉백이 강하면 넉다운
	{
		HitTag = KnockdownHitReactTag;
	}

	FGameplayEventData EventData;
	EventData.EventTag = HitTag;
	EventData.Instigator = InstigatorActor;
	EventData.Target = TargetActor;
	EventData.EventMagnitude = Damage;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(TargetActor, HitTag, EventData);
}

void UAO_GA_AIAttackBase::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UAO_GA_AIAttackBase::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
