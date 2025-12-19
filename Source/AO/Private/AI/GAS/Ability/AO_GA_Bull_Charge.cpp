#include "AI/GAS/Ability/AO_GA_Bull_Charge.h"
#include "AI/Character/AO_Bull.h"
#include "Character/AO_PlayerCharacter.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
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
                // 1. 넉백 적용
                if (UCharacterMovementComponent* MoveComp = Player->GetCharacterMovement())
                {
                    MoveComp->SetMovementMode(MOVE_Falling);
                }
                
                FVector KnockbackDir = Bull->GetActorForwardVector();
                KnockbackDir.Z = 0.1f; // 살짝만 위로 (넉백이 바닥에 박히지 않도록)
                KnockbackDir.Normalize();
                
                Player->LaunchCharacter(KnockbackDir * KnockbackStrength, true, true);

                // 2. 넉다운 이벤트 전송
                SendKnockdownEvent(Player, Bull);
                
                AO_LOG(LogKSJ, Log, TEXT("Bull Charge Hit! Knockdown Applied to %s"), *Player->GetName());
            }
        }
    }
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
