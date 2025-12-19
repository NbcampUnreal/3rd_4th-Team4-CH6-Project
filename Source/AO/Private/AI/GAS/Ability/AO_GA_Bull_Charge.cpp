#include "AI/GAS/Ability/AO_GA_Bull_Charge.h"
#include "AI/Character/AO_Bull.h"
#include "Character/AO_PlayerCharacter.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "AO_Log.h"

UAO_GA_Bull_Charge::UAO_GA_Bull_Charge()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
}

void UAO_GA_Bull_Charge::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AAO_Bull* Bull = Cast<AAO_Bull>(ActorInfo->AvatarActor.Get());
	if (!Bull)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 돌진 상태 활성화
	Bull->SetIsCharging(true);

    // 1. [Troll 참조] 히트 이벤트 대기 (몽타주 Notify에서 Event.Combat.Confirm을 보내야 함)
    UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
        this,
        FGameplayTag::RequestGameplayTag(FName("Event.Combat.Confirm")),
        nullptr,
        false,
        false
    );
    if (WaitEventTask)
    {
        WaitEventTask->EventReceived.AddDynamic(this, &UAO_GA_Bull_Charge::OnHitConfirmEvent);
        WaitEventTask->ReadyForActivation();
    }

	// 2. 돌진 몽타주 재생
	if (ChargeMontage)
	{
		UAbilityTask_PlayMontageAndWait* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			TEXT("Charge"),
			ChargeMontage
		);

		Task->OnBlendOut.AddDynamic(this, &UAO_GA_Bull_Charge::OnMontageCompleted);
		Task->OnCompleted.AddDynamic(this, &UAO_GA_Bull_Charge::OnMontageCompleted);
		Task->OnInterrupted.AddDynamic(this, &UAO_GA_Bull_Charge::OnMontageCancelled);
		Task->OnCancelled.AddDynamic(this, &UAO_GA_Bull_Charge::OnMontageCancelled);

		Task->ReadyForActivation();
	}
	else
	{
		AO_LOG(LogKSJ, Warning, TEXT("Bull Charge: No Montage"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UAO_GA_Bull_Charge::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	AAO_Bull* Bull = Cast<AAO_Bull>(ActorInfo->AvatarActor.Get());
	if (Bull)
	{
		Bull->SetIsCharging(false);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UAO_GA_Bull_Charge::OnHitConfirmEvent(FGameplayEventData Payload)
{
    // [Troll 참조] Sphere Trace 수행 및 Hit 처리
    AAO_Bull* Bull = Cast<AAO_Bull>(GetAvatarActorFromActorInfo());
    if (!Bull) return;

    FVector Start = Bull->GetActorLocation();
    FVector End = Start + (Bull->GetActorForwardVector() * TraceDistance);
    
    TArray<FHitResult> HitResults;
    TArray<AActor*> IgnoreActors;
    IgnoreActors.Add(Bull);

    bool bHit = UKismetSystemLibrary::SphereTraceMulti(
        GetWorld(),
        Start,
        End,
        TraceRadius,
        UEngineTypes::ConvertToTraceType(ECC_Pawn),
        false,
        IgnoreActors,
        EDrawDebugTrace::ForDuration,
        HitResults,
        true
    );

    if (bHit)
    {
        for (const FHitResult& Hit : HitResults)
        {
            if (AAO_PlayerCharacter* Player = Cast<AAO_PlayerCharacter>(Hit.GetActor()))
            {
                // 데미지 및 넉백 적용
                ApplyDamageAndKnockback(Player, Bull);
            }
        }
    }
}

void UAO_GA_Bull_Charge::ApplyDamageAndKnockback(AActor* TargetActor, AActor* InstigatorActor)
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
		DamageSpec.Data.Get()->SetSetByCallerMagnitude(DamageTag, ChargeDamage);
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
		AAO_Bull* Bull = Cast<AAO_Bull>(InstigatorActor);
		if (Bull)
		{
			// 넉백 방향 계산 (Bull에서 타겟 방향)
			FVector KnockbackDirection = (TargetActor->GetActorLocation() - Bull->GetActorLocation()).GetSafeNormal();
			KnockbackDirection.Z = 0.1f;  // 살짝만 위로
			KnockbackDirection.Normalize();

			// 넉백 확실한 적용을 위해 Falling 상태로 강제 전환
			if (UCharacterMovementComponent* MoveComp = TargetCharacter->GetCharacterMovement())
			{
				MoveComp->SetMovementMode(MOVE_Falling);
			}

			// 캐릭터 런치
			TargetCharacter->LaunchCharacter(KnockbackDirection * KnockbackStrength, true, true);
			AO_LOG(LogKSJ, Log, TEXT("ApplyDamageAndKnockback: LaunchCharacter executed"));
		}
	}

	// 넉다운 HitReact 이벤트 발송
	SendKnockdownEvent(TargetActor, InstigatorActor);
	AO_LOG(LogKSJ, Log, TEXT("ApplyDamageAndKnockback: Sent Knockdown Event"));

	AO_LOG(LogKSJ, Log, TEXT("Applied damage %.1f and knockback %.1f to %s"),
		ChargeDamage, KnockbackStrength, *TargetActor->GetName());
}

void UAO_GA_Bull_Charge::SendKnockdownEvent(AActor* TargetActor, AActor* InstigatorActor)
{
    if (!TargetActor) return;

    FGameplayTag HitReactTag = KnockdownHitReactTag;
    if (!HitReactTag.IsValid())
    {
        HitReactTag = FGameplayTag::RequestGameplayTag(FName("Event.Combat.HitReact.Knockdown"));
    }

    FGameplayEventData EventData;
    EventData.EventTag = HitReactTag;
    EventData.Instigator = InstigatorActor;
    EventData.Target = TargetActor;
    EventData.EventMagnitude = ChargeDamage; // 데미지 전달

    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(TargetActor, HitReactTag, EventData);
}

void UAO_GA_Bull_Charge::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UAO_GA_Bull_Charge::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
