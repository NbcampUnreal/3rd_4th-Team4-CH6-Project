// AO_GA_LavaMonster_Attack.cpp

#include "AI/GAS/Ability/AO_GA_LavaMonster_Attack.h"
#include "AI/Character/AO_LavaMonster.h"
#include "Character/AO_PlayerCharacter.h"
#include "Character/Combat/AO_MeleeHitEventPayload.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "AO_Log.h"

UAO_GA_LavaMonster_Attack::UAO_GA_LavaMonster_Attack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
}

void UAO_GA_LavaMonster_Attack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AAO_LavaMonster* LavaMonster = Cast<AAO_LavaMonster>(ActorInfo->AvatarActor.Get());
	if (!LavaMonster)
	{
		AO_LOG(LogKSJ, Warning, TEXT("AO_GA_LavaMonster_Attack: Avatar is not a LavaMonster"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 공격 상태 설정
	LavaMonster->SetIsAttacking(true);

	// 현재 공격 타입 가져오기 (Character에서 설정됨)
	CurrentAttackType = LavaMonster->GetCurrentAttackType();
	CurrentAttackConfig = LavaMonster->GetAttackConfig(CurrentAttackType);

	// 몽타주 확인
	if (!CurrentAttackConfig.AttackMontage)
	{
		AO_LOG(LogKSJ, Warning, TEXT("AO_GA_LavaMonster_Attack: No montage for attack type %d"), static_cast<int32>(CurrentAttackType));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 땅속 범위 공격인 경우 특수 처리
	if (CurrentAttackType == ELavaMonsterAttackType::GroundStrike)
	{
		StartGroundStrike();
	}
	else
	{
		// 일반 공격: 히트 이벤트 대기 Task 먼저 생성 및 활성화
		UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			FGameplayTag::RequestGameplayTag(FName("Event.Combat.Confirm")),
			nullptr,
			false, // 여러 번 발생 가능 (다단 히트 등)
			false
		);

		if (WaitEventTask)
		{
			WaitEventTask->EventReceived.AddDynamic(this, &UAO_GA_LavaMonster_Attack::OnHitConfirmEvent);
			WaitEventTask->ReadyForActivation();
		}
	}

	// 몽타주 재생 Task 생성 및 활성화
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		CurrentAttackConfig.AttackMontage
	);

	if (!MontageTask)
	{
		AO_LOG(LogKSJ, Warning, TEXT("AO_GA_LavaMonster_Attack: Failed to create montage task"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	MontageTask->OnCompleted.AddDynamic(this, &UAO_GA_LavaMonster_Attack::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UAO_GA_LavaMonster_Attack::OnMontageCancelled);
	MontageTask->OnInterrupted.AddDynamic(this, &UAO_GA_LavaMonster_Attack::OnMontageCancelled);

	MontageTask->ReadyForActivation();

	AO_LOG(LogKSJ, Log, TEXT("%s: Started attack type %d"), *LavaMonster->GetName(), static_cast<int32>(CurrentAttackType));
}

void UAO_GA_LavaMonster_Attack::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	// 공격 상태 해제
	if (AAO_LavaMonster* LavaMonster = Cast<AAO_LavaMonster>(ActorInfo->AvatarActor.Get()))
	{
		LavaMonster->SetIsAttacking(false);
	}

	// 땅속 공격 타이머 클리어
	if (GroundStrikeUpdateTimerHandle.IsValid())
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(GroundStrikeUpdateTimerHandle);
		}
	}

	for (auto& TimerPair : GroundStrikeAttackTimers)
	{
		if (TimerPair.Value.IsValid())
		{
			if (UWorld* World = GetWorld())
			{
				World->GetTimerManager().ClearTimer(TimerPair.Value);
			}
		}
	}
	GroundStrikeAttackTimers.Empty();
	GroundStrikeTargets.Empty();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UAO_GA_LavaMonster_Attack::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	AAO_LavaMonster* LavaMonster = Cast<AAO_LavaMonster>(ActorInfo->AvatarActor.Get());
	if (!LavaMonster)
	{
		return false;
	}

	// 기절 중이면 불가
	if (LavaMonster->IsStunned())
	{
		return false;
	}

	// 이미 공격 중이면 불가
	if (LavaMonster->IsAttacking())
	{
		return false;
	}

	// 사용 가능한 공격이 있어야 함
	return LavaMonster->GetAvailableAttackTypes().Num() > 0;
}

void UAO_GA_LavaMonster_Attack::OnHitConfirmEvent(FGameplayEventData Payload)
{
	AO_LOG(LogKSJ, Log, TEXT("AO_GA_LavaMonster_Attack: Received HitConfirmEvent!"));

	const UAO_MeleeHitEventPayload* HitPayload = Cast<UAO_MeleeHitEventPayload>(Payload.OptionalObject);
	if (!HitPayload)
	{
		AO_LOG(LogKSJ, Warning, TEXT("OnHitConfirmEvent: Invalid payload"));
		return;
	}

	AAO_LavaMonster* LavaMonster = Cast<AAO_LavaMonster>(GetAvatarActorFromActorInfo());
	if (!LavaMonster)
	{
		return;
	}

	AO_LOG(LogKSJ, Log, TEXT("AO_GA_LavaMonster_Attack: Performing Trace. Start: %s, End: %s, Radius: %f"), 
		*HitPayload->Params.TraceStart.ToString(), 
		*HitPayload->Params.TraceEnd.ToString(), 
		HitPayload->Params.TraceRadius);

	// 스피어 트레이스 수행 (NotifyState에서 전달된 정보 사용)
	TArray<FHitResult> HitResults;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(LavaMonster);

	const bool bHit = UKismetSystemLibrary::SphereTraceMulti(
		GetWorld(),
		HitPayload->Params.TraceStart,
		HitPayload->Params.TraceEnd,
		CurrentAttackConfig.AttackRadius,
		UEngineTypes::ConvertToTraceType(TraceChannel),
		false,
		ActorsToIgnore,
		EDrawDebugTrace::ForDuration,
		HitResults,
		true,
		FLinearColor::Red,
		FLinearColor::Green,
		2.f
	);

	AO_LOG(LogKSJ, Log, TEXT("Trace Result: Found %d hits"), HitResults.Num());

	if (!bHit)
	{
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

		// 이미 처리한 액터 스킵
		if (ProcessedActors.Contains(HitActor))
		{
			continue;
		}

		// 플레이어 캐릭터만 대상
		AAO_PlayerCharacter* Player = Cast<AAO_PlayerCharacter>(HitActor);
		if (!Player)
		{
			continue;
		}

		ProcessedActors.Add(HitActor);
		
		// 데미지 적용
		ApplyDamageAndKnockback(HitActor, CurrentAttackConfig);
	}
}

void UAO_GA_LavaMonster_Attack::StartGroundStrike()
{
	AAO_LavaMonster* LavaMonster = Cast<AAO_LavaMonster>(GetAvatarActorFromActorInfo());
	if (!LavaMonster)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// 범위 내 모든 플레이어 탐지
	TArray<AActor*> AllPlayers;
	UGameplayStatics::GetAllActorsOfClass(World, AAO_PlayerCharacter::StaticClass(), AllPlayers);

	const FVector LavaMonsterLocation = LavaMonster->GetActorLocation();
	const float StrikeRange = CurrentAttackConfig.GroundStrikeRange;

	GroundStrikeTargets.Empty();

	for (AActor* Actor : AllPlayers)
	{
		AAO_PlayerCharacter* Player = Cast<AAO_PlayerCharacter>(Actor);
		if (!Player)
		{
			continue;
		}

		const float Distance = FVector::Dist(LavaMonsterLocation, Player->GetActorLocation());
		if (Distance <= StrikeRange)
		{
			FAO_GroundStrikeTarget Target;
			Target.Player = Player;
			Target.StrikeLocation = Player->GetActorLocation();
			Target.WarningStartTime = World->GetTimeSeconds();
			Target.bHasStruck = false;

			GroundStrikeTargets.Add(Target);

			AO_LOG(LogKSJ, Log, TEXT("GroundStrike: Found target %s at distance %f"), *Player->GetName(), Distance);
		}
	}

	AO_LOG(LogKSJ, Log, TEXT("GroundStrike: Found %d targets in range"), GroundStrikeTargets.Num());

	// 전조 현상 업데이트 타이머 시작
	if (GroundStrikeTargets.Num() > 0)
	{
		World->GetTimerManager().SetTimer(
			GroundStrikeUpdateTimerHandle,
			this,
			&UAO_GA_LavaMonster_Attack::UpdateGroundStrikeWarning,
			0.1f, // 0.1초마다 업데이트
			true
		);

		// 각 타겟에 대해 공격 타이머 설정
		for (int32 i = 0; i < GroundStrikeTargets.Num(); ++i)
		{
			FTimerHandle AttackTimer;
			FTimerDelegate TimerDelegate;
			TimerDelegate.BindUObject(this, &UAO_GA_LavaMonster_Attack::ExecuteGroundStrikeAtTarget, i);

			World->GetTimerManager().SetTimer(
				AttackTimer,
				TimerDelegate,
				CurrentAttackConfig.WarningDuration,
				false
			);

			GroundStrikeAttackTimers.Add(i, AttackTimer);
		}
	}
}

void UAO_GA_LavaMonster_Attack::UpdateGroundStrikeWarning()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const float CurrentTime = World->GetTimeSeconds();
	const float WarningDuration = CurrentAttackConfig.WarningDuration;

	// 각 타겟 위치에 디버그 범위 표시
	for (const FAO_GroundStrikeTarget& Target : GroundStrikeTargets)
	{
		if (!Target.Player.IsValid() || Target.bHasStruck)
		{
			continue;
		}

		const float ElapsedTime = CurrentTime - Target.WarningStartTime;
		const float WarningProgress = FMath::Clamp(ElapsedTime / WarningDuration, 0.f, 1.f);

		// 빨간색 구체로 전조 현상 표시 (진동 효과를 위해 크기 조절)
		const float SphereRadius = CurrentAttackConfig.AttackRadius * (0.8f + 0.2f * FMath::Sin(ElapsedTime * 10.f));
		const FColor WarningColor = FColor::Red;
		
		DrawDebugSphere(
			World,
			Target.StrikeLocation,
			SphereRadius,
			16,
			WarningColor,
			false,
			0.1f,
			0,
			3.f // 두께
		);
	}
}

void UAO_GA_LavaMonster_Attack::ExecuteGroundStrikeAtTarget(int32 TargetIndex)
{
	if (!GroundStrikeTargets.IsValidIndex(TargetIndex))
	{
		return;
	}

	FAO_GroundStrikeTarget& Target = GroundStrikeTargets[TargetIndex];
	if (Target.bHasStruck || !Target.Player.IsValid())
	{
		return;
	}

	Target.bHasStruck = true;

	// 플레이어가 여전히 해당 위치 근처에 있는지 확인
	AAO_PlayerCharacter* Player = Target.Player.Get();
	if (!Player)
	{
		return;
	}

	const float CurrentDistance = FVector::Dist(Player->GetActorLocation(), Target.StrikeLocation);
	const float AcceptableDistance = CurrentAttackConfig.AttackRadius * 2.f; // 약간의 여유

	if (CurrentDistance <= AcceptableDistance)
	{
		// 데미지 및 넉백 적용
		ApplyDamageAndKnockback(Player, CurrentAttackConfig);

		AO_LOG(LogKSJ, Log, TEXT("GroundStrike: Hit player %s at location %s"), *Player->GetName(), *Target.StrikeLocation.ToString());

		// 디버그: 공격 발동 위치 표시
		if (UWorld* World = GetWorld())
		{
			DrawDebugSphere(
				World,
				Target.StrikeLocation,
				CurrentAttackConfig.AttackRadius,
				16,
				FColor::Yellow,
				false,
				2.f,
				0,
				5.f
			);
		}
	}
	else
	{
		AO_LOG(LogKSJ, Log, TEXT("GroundStrike: Player %s moved away from strike location"), *Player->GetName());
	}

	// 타이머 제거
	if (GroundStrikeAttackTimers.Contains(TargetIndex))
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(GroundStrikeAttackTimers[TargetIndex]);
		}
		GroundStrikeAttackTimers.Remove(TargetIndex);
	}
}

void UAO_GA_LavaMonster_Attack::ApplyDamageAndKnockback(AActor* TargetActor, const FAO_LavaMonsterAttackConfig& Config)
{
	if (!TargetActor || !DamageEffectClass)
	{
		AO_LOG(LogKSJ, Warning, TEXT("ApplyDamageAndKnockback: Invalid Target or DamageEffectClass"));
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
		SourceASC->ApplyGameplayEffectSpecToTarget(*DamageSpec.Data.Get(), TargetASC);
	}

	// 넉백 적용
	ACharacter* TargetCharacter = Cast<ACharacter>(TargetActor);
	if (TargetCharacter && Config.KnockbackStrength > 0.f)
	{
		AAO_LavaMonster* LavaMonster = Cast<AAO_LavaMonster>(GetAvatarActorFromActorInfo());
		if (LavaMonster)
		{
			// 넉백 방향 계산
			FVector KnockbackDirection = (TargetActor->GetActorLocation() - LavaMonster->GetActorLocation()).GetSafeNormal();
			KnockbackDirection.Z = 0.3f; // 약간 위로
			KnockbackDirection.Normalize();

			// 넉백 확실한 적용을 위해 Falling 상태로 강제 전환
			if (UCharacterMovementComponent* MoveComp = TargetCharacter->GetCharacterMovement())
			{
				MoveComp->SetMovementMode(MOVE_Falling);
			}

			// 캐릭터 런치
			TargetCharacter->LaunchCharacter(KnockbackDirection * Config.KnockbackStrength, true, true);
		}
	}

	// HitReact 이벤트 발송
	SendHitReactEvent(TargetActor, GetAvatarActorFromActorInfo());

	AO_LOG(LogKSJ, Log, TEXT("Applied damage %.1f and knockback %.1f to %s"),
		Config.Damage, Config.KnockbackStrength, *TargetActor->GetName());
}

void UAO_GA_LavaMonster_Attack::SendHitReactEvent(AActor* TargetActor, AActor* InstigatorActor)
{
	if (!TargetActor)
	{
		return;
	}

	// 태그가 설정되지 않았으면 기본값 사용
	FGameplayTag EventTag = HitReactTag;
	if (!EventTag.IsValid())
	{
		EventTag = FGameplayTag::RequestGameplayTag(FName("Event.Combat.HitReact.Knockdown"));
	}

	FGameplayEventData EventData;
	EventData.EventTag = EventTag;
	EventData.Instigator = InstigatorActor;
	EventData.Target = TargetActor;
	EventData.EventMagnitude = CurrentAttackConfig.Damage;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(TargetActor, EventTag, EventData);
}

void UAO_GA_LavaMonster_Attack::OnMontageCompleted()
{
	// 땅속 공격인 경우 모든 공격이 완료될 때까지 대기
	if (CurrentAttackType == ELavaMonsterAttackType::GroundStrike)
	{
		// 모든 타겟이 공격되었는지 확인
		bool bAllStruck = true;
		for (const FAO_GroundStrikeTarget& Target : GroundStrikeTargets)
		{
			if (!Target.bHasStruck && Target.Player.IsValid())
			{
				bAllStruck = false;
				break;
			}
		}

		// 아직 공격이 진행 중이면 대기
		if (!bAllStruck)
		{
			return;
		}
	}

	if (CurrentActorInfo)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UAO_GA_LavaMonster_Attack::OnMontageCancelled()
{
	if (CurrentActorInfo)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}
}

