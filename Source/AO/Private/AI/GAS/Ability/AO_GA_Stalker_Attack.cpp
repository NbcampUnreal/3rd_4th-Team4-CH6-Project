//KSJ : AO_GA_Stalker_Attack

#include "AI/GAS/Ability/AO_GA_Stalker_Attack.h"
#include "AI/Character/AO_Stalker.h"
#include "Character/AO_PlayerCharacter.h"
#include "Character/Combat/AO_MeleeHitEventPayload.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"

UAO_GA_Stalker_Attack::UAO_GA_Stalker_Attack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
}

void UAO_GA_Stalker_Attack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AAO_Stalker* Stalker = Cast<AAO_Stalker>(ActorInfo->AvatarActor.Get());
	if (!Stalker)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 공격 상태 설정 (Stalker 클래스에 SetIsAttacking 추가됨)
	Stalker->SetIsAttacking(true);

	UAnimMontage* MontageToPlay = AttackMontage;
	
	if (!MontageToPlay)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 1. 히트 이벤트 대기 Task
	UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		FGameplayTag::RequestGameplayTag(FName("Event.Combat.Confirm")),
		nullptr,
		false, // 여러 번 발생 가능
		false
	);

	if (WaitEventTask)
	{
		WaitEventTask->EventReceived.AddDynamic(this, &UAO_GA_Stalker_Attack::OnHitConfirmEvent);
		WaitEventTask->ReadyForActivation();
	}

	// 2. 몽타주 재생
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		TEXT("Ambush"),
		MontageToPlay
	);

	if (MontageTask)
	{
		MontageTask->OnCompleted.AddDynamic(this, &UAO_GA_Stalker_Attack::OnMontageCompleted);
		MontageTask->OnBlendOut.AddDynamic(this, &UAO_GA_Stalker_Attack::OnMontageCompleted);
		MontageTask->OnInterrupted.AddDynamic(this, &UAO_GA_Stalker_Attack::OnMontageCancelled);
		MontageTask->OnCancelled.AddDynamic(this, &UAO_GA_Stalker_Attack::OnMontageCancelled);
		MontageTask->ReadyForActivation();
	}
}

void UAO_GA_Stalker_Attack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (AAO_Stalker* Stalker = Cast<AAO_Stalker>(ActorInfo->AvatarActor.Get()))
	{
		// 공격 상태 해제
		Stalker->SetIsAttacking(false);

		// 공격 성공/완료 시 후퇴 모드 전환
		if (!bWasCancelled)
		{
			Stalker->SetRetreatMode(true);
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UAO_GA_Stalker_Attack::OnHitConfirmEvent(FGameplayEventData Payload)
{
	const UAO_MeleeHitEventPayload* HitPayload = Cast<UAO_MeleeHitEventPayload>(Payload.OptionalObject);
	if (!HitPayload)
	{
		return;
	}

	AAO_Stalker* Stalker = Cast<AAO_Stalker>(GetAvatarActorFromActorInfo());
	if (!Stalker)
	{
		return;
	}

	// 스피어 트레이스 수행
	TArray<FHitResult> HitResults;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(Stalker);

	const bool bHit = UKismetSystemLibrary::SphereTraceMulti(
		GetWorld(),
		HitPayload->Params.TraceStart,
		HitPayload->Params.TraceEnd,
		HitPayload->Params.TraceRadius,
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

	if (!bHit)
	{
		return;
	}

	// 중복 제거
	TSet<AActor*> ProcessedActors;

	for (const FHitResult& Hit : HitResults)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor || ProcessedActors.Contains(HitActor))
		{
			continue;
		}

		if (AAO_PlayerCharacter* Player = Cast<AAO_PlayerCharacter>(HitActor))
		{
			ProcessedActors.Add(HitActor);
			ApplyDamageAndKnockback(HitActor);
		}
	}
}

void UAO_GA_Stalker_Attack::ApplyDamageAndKnockback(AActor* TargetActor)
{
	if (!TargetActor)
	{
		return;
	}

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);

	if (!SourceASC || !TargetASC)
	{
		return;
	}

	// 무적 상태 확인
	const FGameplayTag InvulnerableTag = FGameplayTag::RequestGameplayTag(FName("Status.Invulnerable"));
	if (TargetASC->HasMatchingGameplayTag(InvulnerableTag))
	{
		return;
	}

	// 타격음 재생 (무적 체크 통과 후, 실제로 맞았을 때만)
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			HitSound,
			TargetActor->GetActorLocation(),
			1.f,
			1.f,
			0.f,
			HitSoundAttenuation
		);
	}

	// 1. 데미지 적용
	if (DamageEffectClass)
	{
		FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
		Context.AddInstigator(GetOwningActorFromActorInfo(), GetAvatarActorFromActorInfo());

		FGameplayEffectSpecHandle DamageSpec = SourceASC->MakeOutgoingSpec(DamageEffectClass, 1.f, Context);
		if (DamageSpec.IsValid())
		{
			const FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(FName("Data.Damage"));
			DamageSpec.Data.Get()->SetSetByCallerMagnitude(DamageTag, AttackDamage);
			SourceASC->ApplyGameplayEffectSpecToTarget(*DamageSpec.Data.Get(), TargetASC);
		}
	}

	// 2. HitReact 이벤트 발송
	SendHitReactEvent(TargetActor, GetAvatarActorFromActorInfo());

	// 3. 넉백 적용
	ACharacter* TargetCharacter = Cast<ACharacter>(TargetActor);
	if (TargetCharacter && KnockbackStrength > 0.f)
	{
		AAO_Stalker* Stalker = Cast<AAO_Stalker>(GetAvatarActorFromActorInfo());
		if (Stalker)
		{
			FVector KnockbackDirection = (TargetActor->GetActorLocation() - Stalker->GetActorLocation()).GetSafeNormal();
			KnockbackDirection.Z = 0.2f; // 약간 위로
			KnockbackDirection.Normalize();

			if (UCharacterMovementComponent* MoveComp = TargetCharacter->GetCharacterMovement())
			{
				MoveComp->SetMovementMode(MOVE_Falling);
			}
			TargetCharacter->LaunchCharacter(KnockbackDirection * KnockbackStrength, true, true);
		}
	}
}

void UAO_GA_Stalker_Attack::SendHitReactEvent(AActor* TargetActor, AActor* InstigatorActor)
{
	if (!TargetActor)
	{
		return;
	}

	FGameplayTag TagToSend = HitReactTag;
	if (!TagToSend.IsValid())
	{
		// 기본 태그 (설정 안 했을 경우)
		TagToSend = FGameplayTag::RequestGameplayTag(FName("Event.Combat.HitReact.Light"));
	}

	FGameplayEventData EventData;
	EventData.EventTag = TagToSend;
	EventData.Instigator = InstigatorActor;
	EventData.Target = TargetActor;
	EventData.EventMagnitude = AttackDamage;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(TargetActor, TagToSend, EventData);
}

void UAO_GA_Stalker_Attack::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UAO_GA_Stalker_Attack::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
