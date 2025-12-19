#include "AI/GAS/Ability/AO_GA_AIAttackBase.h"
#include "Character/AO_PlayerCharacter.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AO_Log.h"

UAO_GA_AIAttackBase::UAO_GA_AIAttackBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
}

void UAO_GA_AIAttackBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

    // 히트 이벤트 대기 (몽타주 Notify에서 Event.Combat.Confirm을 보내야 함)
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

    UAnimMontage* MontageToPlay = GetMontageToPlay();
	if (MontageToPlay)
	{
		UAbilityTask_PlayMontageAndWait* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			MontageToPlay
		);

		Task->OnBlendOut.AddDynamic(this, &UAO_GA_AIAttackBase::OnMontageCompleted);
		Task->OnCompleted.AddDynamic(this, &UAO_GA_AIAttackBase::OnMontageCompleted);
		Task->OnInterrupted.AddDynamic(this, &UAO_GA_AIAttackBase::OnMontageCancelled);
		Task->OnCancelled.AddDynamic(this, &UAO_GA_AIAttackBase::OnMontageCancelled);

		Task->ReadyForActivation();
	}
	else
	{
		AO_LOG(LogKSJ, Warning, TEXT("AO_GA_AIAttackBase: No Montage to play"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UAO_GA_AIAttackBase::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

UAnimMontage* UAO_GA_AIAttackBase::GetMontageToPlay() const
{
    return AttackMontage;
}

void UAO_GA_AIAttackBase::OnHitConfirmEvent(FGameplayEventData Payload)
{
    AActor* AvatarActor = GetAvatarActorFromActorInfo();
    if (!AvatarActor) return;

    FVector Start = AvatarActor->GetActorLocation();
    FVector End = Start + (AvatarActor->GetActorForwardVector() * TraceDistance);
    
    TArray<FHitResult> HitResults;
    TArray<AActor*> IgnoreActors;
    IgnoreActors.Add(AvatarActor);

    bool bHit = UKismetSystemLibrary::SphereTraceMulti(
        GetWorld(),
        Start,
        End,
        TraceRadius,
        UEngineTypes::ConvertToTraceType(ECC_Pawn),
        false,
        IgnoreActors,
        bShowDebugTrace ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
        HitResults,
        true,
        FLinearColor::Red,
        FLinearColor::Green,
        1.0f
    );

    if (bHit)
    {
        // 중복 피격 방지
        TSet<AActor*> HitActors;

        for (const FHitResult& Hit : HitResults)
        {
            AActor* HitActor = Hit.GetActor();
            if (HitActor && !HitActors.Contains(HitActor))
            {
                // 플레이어만 타격 (필요 시 팀 체크 추가 가능)
                if (Cast<AAO_PlayerCharacter>(HitActor))
                {
                    HitActors.Add(HitActor);
                    OnTargetHit(HitActor, AvatarActor);
                }
            }
        }
    }
}

void UAO_GA_AIAttackBase::OnTargetHit(AActor* TargetActor, AActor* InstigatorActor)
{
    if (!TargetActor || !InstigatorActor) return;

    UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
    UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);

    // 1. 데미지 GE 적용
    if (DamageEffectClass && SourceASC && TargetASC)
    {
        // 무적 확인 등은 여기서 생략 (필요 시 추가)
        FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
        Context.AddInstigator(InstigatorActor, InstigatorActor);

        FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, 1.f, Context);
        if (SpecHandle.IsValid())
        {
            // SetByCaller로 데미지 전달
            FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(FName("Data.Damage"));
            SpecHandle.Data.Get()->SetSetByCallerMagnitude(DamageTag, DamageAmount);
            
            SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
        }
    }

    // 2. 넉백 적용
    if (KnockbackStrength > 0.f)
    {
        ACharacter* TargetChar = Cast<ACharacter>(TargetActor);
        if (TargetChar)
        {
            if (UCharacterMovementComponent* MoveComp = TargetChar->GetCharacterMovement())
            {
                MoveComp->SetMovementMode(MOVE_Falling);
            }
            
            FVector KnockbackDir = InstigatorActor->GetActorForwardVector();
            KnockbackDir.Z = KnockbackUpForce;
            KnockbackDir.Normalize();
            
            TargetChar->LaunchCharacter(KnockbackDir * KnockbackStrength, true, true);
        }
    }

    // 3. HitReact 이벤트 전송 (넉다운, 경직 등)
    if (HitReactTag.IsValid())
    {
        FGameplayEventData EventData;
        EventData.EventTag = HitReactTag;
        EventData.Instigator = InstigatorActor;
        EventData.Target = TargetActor;
        EventData.EventMagnitude = DamageAmount;

        UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(TargetActor, HitReactTag, EventData);
    }
}

void UAO_GA_AIAttackBase::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UAO_GA_AIAttackBase::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

