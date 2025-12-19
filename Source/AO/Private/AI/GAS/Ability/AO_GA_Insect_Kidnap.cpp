// AO_GA_Insect_Kidnap.cpp

#include "AI/GAS/Ability/AO_GA_Insect_Kidnap.h"
#include "AI/Character/AO_Insect.h"
#include "AI/Component/AO_KidnapComponent.h"
#include "AI/Subsystem/AO_AISubsystem.h"
#include "Character/AO_PlayerCharacter.h"
#include "Character/Combat/AO_MeleeHitEventPayload.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AO_Log.h"

UAO_GA_Insect_Kidnap::UAO_GA_Insect_Kidnap()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
}

bool UAO_GA_Insect_Kidnap::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	AAO_Insect* Insect = Cast<AAO_Insect>(ActorInfo->AvatarActor.Get());
	if (!Insect || Insect->IsKidnapping() || Insect->IsStunned())
	{
		return false;
	}

	return true;
}

void UAO_GA_Insect_Kidnap::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		AO_LOG(LogKSJ, Warning, TEXT("AO_GA_Insect_Kidnap: Failed to commit ability"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AAO_Insect* Insect = Cast<AAO_Insect>(ActorInfo->AvatarActor.Get());
	if (!Insect)
	{
		AO_LOG(LogKSJ, Error, TEXT("AO_GA_Insect_Kidnap: Avatar is not an Insect"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!KidnapMontage)
	{
		AO_LOG(LogKSJ, Warning, TEXT("AO_GA_Insect_Kidnap: No Montage Assigned"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AO_LOG(LogKSJ, Log, TEXT("AO_GA_Insect_Kidnap: Activating kidnap ability"));

	// 1. 히트 이벤트 대기 (몽타주 Notify에서 발송될 수 있음)
	UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		FGameplayTag::RequestGameplayTag(FName("Event.Combat.Confirm")),
		nullptr,
		false,
		false
	);
	if (WaitEventTask)
	{
		WaitEventTask->EventReceived.AddDynamic(this, &UAO_GA_Insect_Kidnap::OnHitConfirmEvent);
		WaitEventTask->ReadyForActivation();
		AO_LOG(LogKSJ, Log, TEXT("AO_GA_Insect_Kidnap: Waiting for Event.Combat.Confirm"));
	}

	// 2. 몽타주 재생
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		KidnapMontage
	);
	if (MontageTask)
	{
		MontageTask->OnCompleted.AddDynamic(this, &UAO_GA_Insect_Kidnap::OnMontageCompleted);
		MontageTask->OnCancelled.AddDynamic(this, &UAO_GA_Insect_Kidnap::OnMontageCancelled);
		MontageTask->OnInterrupted.AddDynamic(this, &UAO_GA_Insect_Kidnap::OnMontageCancelled);
		MontageTask->ReadyForActivation();
		AO_LOG(LogKSJ, Log, TEXT("AO_GA_Insect_Kidnap: Playing montage: %s"), *KidnapMontage->GetName());
	}
	else
	{
		AO_LOG(LogKSJ, Error, TEXT("AO_GA_Insect_Kidnap: Failed to create montage task"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 3. 몽타주 재생 중 주기적으로 Trace 수행 (이벤트가 없어도 작동하도록)
	// 몽타주 길이의 중간 지점에서 Trace 수행
	if (KidnapMontage && KidnapMontage->GetPlayLength() > 0.f)
	{
		float TraceDelay = KidnapMontage->GetPlayLength() * 0.5f; // 몽타주 중간 지점
		GetWorld()->GetTimerManager().SetTimer(
			TraceTimerHandle,
			this,
			&UAO_GA_Insect_Kidnap::PerformKidnapTrace,
			TraceDelay,
			false
		);
		AO_LOG(LogKSJ, Log, TEXT("AO_GA_Insect_Kidnap: Scheduled trace at %.2f seconds"), TraceDelay);
	}
}

void UAO_GA_Insect_Kidnap::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 타이머 정리
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TraceTimerHandle);
	}

	AO_LOG(LogKSJ, Log, TEXT("AO_GA_Insect_Kidnap: EndAbility (Cancelled: %s)"), bWasCancelled ? TEXT("true") : TEXT("false"));
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UAO_GA_Insect_Kidnap::OnHitConfirmEvent(FGameplayEventData Payload)
{
	AO_LOG(LogKSJ, Log, TEXT("AO_GA_Insect_Kidnap: OnHitConfirmEvent received"));
	PerformKidnapTrace();
}

void UAO_GA_Insect_Kidnap::PerformKidnapTrace()
{
	AAO_Insect* Insect = Cast<AAO_Insect>(GetAvatarActorFromActorInfo());
	if (!Insect)
	{
		AO_LOG(LogKSJ, Warning, TEXT("AO_GA_Insect_Kidnap::PerformKidnapTrace: Insect is null"));
		return;
	}

	// 이미 납치 중이면 스킵
	if (Insect->IsKidnapping())
	{
		AO_LOG(LogKSJ, Log, TEXT("AO_GA_Insect_Kidnap::PerformKidnapTrace: Already kidnapping, skipping"));
		return;
	}

	// 전방 Sphere Trace
	FVector Start = Insect->GetActorLocation();
	FVector Forward = Insect->GetActorForwardVector();
	FVector End = Start + Forward * TraceDistance;

	TArray<FHitResult> HitResults;
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(Insect);

	AO_LOG(LogKSJ, Log, TEXT("AO_GA_Insect_Kidnap::PerformKidnapTrace: Trace from %s to %s (Radius: %.1f, Distance: %.1f)"),
		*Start.ToString(), *End.ToString(), TraceRadius, TraceDistance);

	bool bHit = UKismetSystemLibrary::SphereTraceMulti(
		GetWorld(),
		Start,
		End,
		TraceRadius,
		UEngineTypes::ConvertToTraceType(TraceChannel),
		false,
		IgnoreActors,
		EDrawDebugTrace::ForDuration,
		HitResults,
		true
	);

	if (!bHit)
	{
		AO_LOG(LogKSJ, Warning, TEXT("AO_GA_Insect_Kidnap::PerformKidnapTrace: No hit detected"));
		return;
	}

	AO_LOG(LogKSJ, Log, TEXT("AO_GA_Insect_Kidnap::PerformKidnapTrace: Hit %d actors"), HitResults.Num());

	for (const FHitResult& Hit : HitResults)
	{
		if (AAO_PlayerCharacter* Player = Cast<AAO_PlayerCharacter>(Hit.GetActor()))
		{
			AO_LOG(LogKSJ, Log, TEXT("AO_GA_Insect_Kidnap::PerformKidnapTrace: Found player %s"), *Player->GetName());
			
			// 빠른 체크: 이미 납치된 플레이어는 건너뛰기
			UAbilitySystemComponent* PlayerASC = Player->GetAbilitySystemComponent();
			if (PlayerASC)
			{
				const FGameplayTag KidnappedTag = FGameplayTag::RequestGameplayTag(FName("Status.Debuff.Kidnapped"));
				if (PlayerASC->HasMatchingGameplayTag(KidnappedTag))
				{
					AO_LOG(LogKSJ, Log, TEXT("AO_GA_Insect_Kidnap::PerformKidnapTrace: Player %s is already kidnapped, skipping"), *Player->GetName());
					continue;
				}
			}
			
			// AISubsystem 체크: 이미 다른 Insect가 예약했는지 확인
			if (UWorld* World = GetWorld())
			{
				if (UAO_AISubsystem* AISubsystem = World->GetSubsystem<UAO_AISubsystem>())
				{
					if (AISubsystem->IsPlayerBeingKidnapped(Player))
					{
						AO_LOG(LogKSJ, Log, TEXT("AO_GA_Insect_Kidnap::PerformKidnapTrace: Player %s is already reserved by another Insect, skipping"), *Player->GetName());
						continue;
					}
				}
			}
			
			// 납치 시도
			if (Insect->GetKidnapComponent()->TryKidnapPlayer(Player))
			{
				AO_LOG(LogKSJ, Log, TEXT("AO_GA_Insect_Kidnap: Kidnap Successful!"));
				// 성공했으므로 타이머 취소
				GetWorld()->GetTimerManager().ClearTimer(TraceTimerHandle);
				break; // 한 명만 납치
			}
			else
			{
				AO_LOG(LogKSJ, Warning, TEXT("AO_GA_Insect_Kidnap::PerformKidnapTrace: TryKidnapPlayer failed for %s"), *Player->GetName());
			}
		}
	}
}

void UAO_GA_Insect_Kidnap::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UAO_GA_Insect_Kidnap::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

