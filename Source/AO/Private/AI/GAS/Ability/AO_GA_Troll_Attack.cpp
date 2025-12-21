// AO_GA_Troll_Attack.cpp

#include "AI/GAS/Ability/AO_GA_Troll_Attack.h"
#include "AI/Character/AO_Troll.h"
#include "Character/AO_PlayerCharacter.h"
#include "Character/Combat/AO_MeleeHitEventPayload.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AO_Log.h"

UAO_GA_Troll_Attack::UAO_GA_Troll_Attack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	// 기본 넉다운 태그 - CDO 생성자에서는 RequestGameplayTag 사용 불가
	// 에디터에서 설정하거나 ActivateAbility에서 초기화
}

void UAO_GA_Troll_Attack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AAO_Troll* Troll = Cast<AAO_Troll>(ActorInfo->AvatarActor.Get());
	if (!Troll)
	{
		AO_LOG(LogKSJ, Warning, TEXT("AO_GA_Troll_Attack: Avatar is not a Troll"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 공격 상태 설정
	Troll->SetIsAttacking(true);

	// 랜덤 공격 타입 선택
	CurrentAttackType = Troll->SelectRandomAttackType();
	CurrentAttackConfig = Troll->GetAttackConfig(CurrentAttackType);

	// 몽타주 확인
	if (!CurrentAttackConfig.AttackMontage)
	{
		AO_LOG(LogKSJ, Warning, TEXT("AO_GA_Troll_Attack: No montage for attack type %d"), static_cast<int32>(CurrentAttackType));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 1. 히트 이벤트 대기 Task 먼저 생성 및 활성화 (NotifyState 이벤트를 놓치지 않기 위해)
	UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		FGameplayTag::RequestGameplayTag(FName("Event.Combat.Confirm")),
		nullptr,
		false, // 여러 번 발생 가능 (다단 히트 등)
		false
	);

	if (WaitEventTask)
	{
		WaitEventTask->EventReceived.AddDynamic(this, &UAO_GA_Troll_Attack::OnHitConfirmEvent);
		WaitEventTask->ReadyForActivation();
	}

	// 2. 몽타주 재생 Task 생성 및 활성화
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		CurrentAttackConfig.AttackMontage
	);

	if (!MontageTask)
	{
		AO_LOG(LogKSJ, Warning, TEXT("AO_GA_Troll_Attack: Failed to create montage task"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	MontageTask->OnCompleted.AddDynamic(this, &UAO_GA_Troll_Attack::OnMontageCompleted);
	// OnBlendOut 시점에 종료하면 NotifyEnd 이벤트를 못 받을 수 있으므로 주석 처리
	// MontageTask->OnBlendOut.AddDynamic(this, &UAO_GA_Troll_Attack::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UAO_GA_Troll_Attack::OnMontageCancelled);
	MontageTask->OnInterrupted.AddDynamic(this, &UAO_GA_Troll_Attack::OnMontageCancelled);

	MontageTask->ReadyForActivation();

	AO_LOG(LogKSJ, Log, TEXT("%s: Started attack type %d"), *Troll->GetName(), static_cast<int32>(CurrentAttackType));
}

void UAO_GA_Troll_Attack::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	// 공격 상태 해제
	if (AAO_Troll* Troll = Cast<AAO_Troll>(ActorInfo->AvatarActor.Get()))
	{
		Troll->SetIsAttacking(false);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UAO_GA_Troll_Attack::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	AAO_Troll* Troll = Cast<AAO_Troll>(ActorInfo->AvatarActor.Get());
	if (!Troll)
	{
		return false;
	}

	// 기절 중이면 불가
	if (Troll->IsStunned())
	{
		return false;
	}

	// 이미 공격 중이면 불가
	if (Troll->IsAttacking())
	{
		return false;
	}

	// 사용 가능한 공격이 있어야 함
	return Troll->GetAvailableAttackTypes().Num() > 0;
}

void UAO_GA_Troll_Attack::OnHitConfirmEvent(FGameplayEventData Payload)
{
	AO_LOG(LogKSJ, Log, TEXT("AO_GA_Troll_Attack: Received HitConfirmEvent!"));

	const UAO_MeleeHitEventPayload* HitPayload = Cast<UAO_MeleeHitEventPayload>(Payload.OptionalObject);
	if (!HitPayload)
	{
		AO_LOG(LogKSJ, Warning, TEXT("OnHitConfirmEvent: Invalid payload"));
		return;
	}

	AAO_Troll* Troll = Cast<AAO_Troll>(GetAvatarActorFromActorInfo());
	if (!Troll)
	{
		return;
	}

	AO_LOG(LogKSJ, Log, TEXT("AO_GA_Troll_Attack: Performing Trace. Start: %s, End: %s, Radius: %f"), 
		*HitPayload->Params.TraceStart.ToString(), 
		*HitPayload->Params.TraceEnd.ToString(), 
		HitPayload->Params.TraceRadius);

	// 스피어 트레이스 수행 (NotifyState에서 전달된 정보 사용)
	TArray<FHitResult> HitResults;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(Troll);

	const bool bHit = UKismetSystemLibrary::SphereTraceMulti(
		GetWorld(),
		HitPayload->Params.TraceStart,
		HitPayload->Params.TraceEnd,
		HitPayload->Params.TraceRadius,
		UEngineTypes::ConvertToTraceType(TraceChannel),
		false,
		ActorsToIgnore,
		EDrawDebugTrace::ForDuration, // 디버깅용 (필요 시 None으로 변경)
		HitResults,
		true,
		FLinearColor::Red,
		FLinearColor::Green,
		2.f
	);

	// 감지된 갯수 로그
	AO_LOG(LogKSJ, Log, TEXT("Trace Result: Found %d hits"), HitResults.Num());

	if (!bHit)
	{
		AO_LOG(LogKSJ, Log, TEXT("Trace Result: No hits detected"));
		return;
	}

	// 중복 제거를 위한 Set
	TSet<TWeakObjectPtr<AActor>> ProcessedActors;

	for (const FHitResult& Hit : HitResults)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor)
		{
			continue;
		}

		// 감지된 액터 이름 로그
		AO_LOG(LogKSJ, Log, TEXT("Hit Actor: %s"), *HitActor->GetName());

		// 이미 처리한 액터 스킵
		if (ProcessedActors.Contains(HitActor))
		{
			continue;
		}

		// 플레이어 캐릭터만 대상
		AAO_PlayerCharacter* Player = Cast<AAO_PlayerCharacter>(HitActor);
		if (!Player)
		{
			AO_LOG(LogKSJ, Log, TEXT("  -> Not a Player Character"));
			continue;
		}

		ProcessedActors.Add(HitActor);
		
		// ApplyDamageAndKnockback 호출 직전 로그
		AO_LOG(LogKSJ, Log, TEXT("  -> Applying Damage to Player!")); 
		
		// 데미지 적용 (CurrentAttackConfig 사용)
		ApplyDamageAndKnockback(HitActor, CurrentAttackConfig);
	}
}

void UAO_GA_Troll_Attack::ApplyDamageAndKnockback(AActor* TargetActor, const FAO_TrollAttackConfig& Config)
{
	if (!TargetActor || !DamageEffectClass)
	{
		AO_LOG(LogKSJ, Warning, TEXT("ApplyDamageAndKnockback: Invalid Target or DamageEffectClass (Target: %s, DamageClass: %s)"), 
			TargetActor ? *TargetActor->GetName() : TEXT("Null"), 
			DamageEffectClass ? *DamageEffectClass->GetName() : TEXT("Null"));
		return;
	}

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);

	if (!SourceASC || !TargetASC)
	{
		AO_LOG(LogKSJ, Warning, TEXT("ApplyDamageAndKnockback: Missing ASC (Source: %s, Target: %s)"), 
			SourceASC ? *SourceASC->GetName() : TEXT("Null"), 
			TargetASC ? *TargetASC->GetName() : TEXT("Null"));
		return;
	}

	// 무적 상태 확인
	const FGameplayTag InvulnerableTag = FGameplayTag::RequestGameplayTag(FName("Status.Invulnerable"));
	if (TargetASC->HasMatchingGameplayTag(InvulnerableTag))
	{
		AO_LOG(LogKSJ, Log, TEXT("ApplyDamageAndKnockback: Target is Invulnerable"));
		return;
	}

	// 데미지 적용
	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	Context.AddInstigator(GetOwningActorFromActorInfo(), GetAvatarActorFromActorInfo());

	FGameplayEffectSpecHandle DamageSpec = SourceASC->MakeOutgoingSpec(DamageEffectClass, 1.f, Context);
	if (DamageSpec.IsValid())
	{
		const FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(FName("Data.Damage"));
		DamageSpec.Data.Get()->SetSetByCallerMagnitude(DamageTag, Config.Damage);
		FActiveGameplayEffectHandle ActiveGE = SourceASC->ApplyGameplayEffectSpecToTarget(*DamageSpec.Data.Get(), TargetASC);
		
		AO_LOG(LogKSJ, Log, TEXT("ApplyDamageAndKnockback: Applied GE_Damage. Spec Valid: True. ActiveGE Valid: %s"), ActiveGE.WasSuccessfullyApplied() ? TEXT("True") : TEXT("False"));
	}
	else
	{
		AO_LOG(LogKSJ, Warning, TEXT("ApplyDamageAndKnockback: DamageSpec is Invalid!"));
	}

	// 넉백 적용
	ACharacter* TargetCharacter = Cast<ACharacter>(TargetActor);
	if (TargetCharacter)
	{
		AAO_Troll* Troll = Cast<AAO_Troll>(GetAvatarActorFromActorInfo());
		if (Troll)
		{
			// 넉백 방향 계산 (Troll에서 타겟 방향)
			FVector KnockbackDirection = (TargetActor->GetActorLocation() - Troll->GetActorLocation()).GetSafeNormal();
			KnockbackDirection.Z = 0.3f;  // 약간 위로
			KnockbackDirection.Normalize();

			// 넉백 확실한 적용을 위해 Falling 상태로 강제 전환
			if (UCharacterMovementComponent* MoveComp = TargetCharacter->GetCharacterMovement())
			{
				MoveComp->SetMovementMode(MOVE_Falling);
			}

			// 캐릭터 런치
			TargetCharacter->LaunchCharacter(KnockbackDirection * Config.KnockbackStrength, true, true);
			AO_LOG(LogKSJ, Log, TEXT("ApplyDamageAndKnockback: LaunchCharacter executed"));
		}
	}

	// 넉다운 HitReact 이벤트 발송
	SendKnockdownEvent(TargetActor, GetAvatarActorFromActorInfo());
	AO_LOG(LogKSJ, Log, TEXT("ApplyDamageAndKnockback: Sent Knockdown Event"));

	AO_LOG(LogKSJ, Log, TEXT("Applied damage %.1f and knockback %.1f to %s"),
		Config.Damage, Config.KnockbackStrength, *TargetActor->GetName());
}

void UAO_GA_Troll_Attack::SendKnockdownEvent(AActor* TargetActor, AActor* InstigatorActor)
{
	if (!TargetActor)
	{
		return;
	}

	// 태그가 설정되지 않았으면 기본값 사용
	FGameplayTag HitReactTag = KnockdownHitReactTag;
	if (!HitReactTag.IsValid())
	{
		HitReactTag = FGameplayTag::RequestGameplayTag(FName("Event.Combat.HitReact.Knockdown"));
	}

	FGameplayEventData EventData;
	EventData.EventTag = HitReactTag;
	EventData.Instigator = InstigatorActor;
	EventData.Target = TargetActor;
	EventData.EventMagnitude = CurrentAttackConfig.Damage;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(TargetActor, HitReactTag, EventData);
}

void UAO_GA_Troll_Attack::OnMontageCompleted()
{
	if (CurrentActorInfo)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UAO_GA_Troll_Attack::OnMontageCancelled()
{
	if (CurrentActorInfo)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}
}
