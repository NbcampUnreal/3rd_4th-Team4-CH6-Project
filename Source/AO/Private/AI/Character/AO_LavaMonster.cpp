// AO_LavaMonster.cpp

#include "AI/Character/AO_LavaMonster.h"
#include "AI/Animation/AO_LavaMonster_AnimInstance.h"
#include "Character/AO_PlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "AI/GAS/Ability/AO_GA_LavaMonster_Attack.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AO_Log.h"

AAO_LavaMonster::AAO_LavaMonster()
{
	// 용암 몬스터 속도 설정
	RoamSpeed = 300.f;
	ChaseSpeed = 500.f;
	DefaultMovementSpeed = RoamSpeed;
	AlertMovementSpeed = ChaseSpeed;

	// 기본 공격 범위
	AttackRange = 250.f;
}

void AAO_LavaMonster::BeginPlay()
{
	Super::BeginPlay();

	// 기본 공격 설정이 없으면 초기화
	if (AttackConfigs.Num() == 0)
	{
		// 근접 주먹 지르기
		FAO_LavaMonsterAttackConfig MeleePunch;
		MeleePunch.Damage = 25.f;
		MeleePunch.KnockbackStrength = 400.f;
		MeleePunch.AttackRadius = 150.f;
		MeleePunch.AttackDistance = 200.f;
		AttackConfigs.Add(ELavaMonsterAttackType::MeleePunch, MeleePunch);

		// 근접 어퍼컷
		FAO_LavaMonsterAttackConfig MeleeUppercut;
		MeleeUppercut.Damage = 30.f;
		MeleeUppercut.KnockbackStrength = 500.f;
		MeleeUppercut.AttackRadius = 150.f;
		MeleeUppercut.AttackDistance = 200.f;
		AttackConfigs.Add(ELavaMonsterAttackType::MeleeUppercut, MeleeUppercut);

		// 중거리 채찍 공격
		FAO_LavaMonsterAttackConfig WhipMidRange;
		WhipMidRange.Damage = 20.f;
		WhipMidRange.KnockbackStrength = 350.f;
		WhipMidRange.AttackRadius = 100.f;
		WhipMidRange.AttackDistance = 400.f;
		AttackConfigs.Add(ELavaMonsterAttackType::WhipMidRange, WhipMidRange);

		// 중~원거리 팔 늘리기
		FAO_LavaMonsterAttackConfig ExtendArm;
		ExtendArm.Damage = 25.f;
		ExtendArm.KnockbackStrength = 400.f;
		ExtendArm.AttackRadius = 120.f;
		ExtendArm.AttackDistance = 600.f;
		AttackConfigs.Add(ELavaMonsterAttackType::ExtendArm, ExtendArm);

		// 땅속 범위 공격
		FAO_LavaMonsterAttackConfig GroundStrike;
		GroundStrike.Damage = 35.f;
		GroundStrike.KnockbackStrength = 600.f;
		GroundStrike.AttackRadius = 200.f; // 각 플레이어 발밑 공격 반경
		GroundStrike.AttackDistance = 0.f; // 사용 안 함
		GroundStrike.GroundStrikeRange = 1000.f; // 범위 탐지 반경
		GroundStrike.WarningDuration = 2.f; // 전조 현상 지속 시간
		AttackConfigs.Add(ELavaMonsterAttackType::GroundStrike, GroundStrike);
	}
}

ELavaMonsterAttackType AAO_LavaMonster::SelectRandomAttackType() const
{
	TArray<ELavaMonsterAttackType> AvailableTypes = GetAvailableAttackTypes();
	
	if (AvailableTypes.Num() == 0)
	{
		// 사용 가능한 공격이 없으면 기본값
		return ELavaMonsterAttackType::MeleePunch;
	}

	const int32 RandomIndex = FMath::RandRange(0, AvailableTypes.Num() - 1);
	return AvailableTypes[RandomIndex];
}

TArray<ELavaMonsterAttackType> AAO_LavaMonster::GetAvailableAttackTypes() const
{
	TArray<ELavaMonsterAttackType> AvailableTypes;

	for (const auto& Pair : AttackConfigs)
	{
		// 모든 공격 타입 사용 가능 (무기 조건 없음)
		AvailableTypes.Add(Pair.Key);
	}

	return AvailableTypes;
}

TArray<ELavaMonsterAttackType> AAO_LavaMonster::GetAvailableAttackTypesByRange(float Distance) const
{
	TArray<ELavaMonsterAttackType> AvailableTypes;
	
	if (!CurrentTarget.IsValid())
	{
		return AvailableTypes;
	}

	for (const auto& Pair : AttackConfigs)
	{
		const FAO_LavaMonsterAttackConfig& Config = Pair.Value;
		const ELavaMonsterAttackType AttackType = Pair.Key;
		
		// 땅속 공격은 범위 공격이므로 GroundStrikeRange 내에서만 사용 가능
		if (AttackType == ELavaMonsterAttackType::GroundStrike)
		{
			if (Distance <= Config.GroundStrikeRange)
			{
				AvailableTypes.Add(AttackType);
			}
			continue;
		}
		
		// 일반 공격은 AttackDistance 범위 내에서만 사용 가능
		// 근접 공격은 최소 거리 없음, 중거리 이상은 최소 거리 필요
		float MinRange = 0.f;
		if (AttackType == ELavaMonsterAttackType::WhipMidRange || 
		    AttackType == ELavaMonsterAttackType::ExtendArm)
		{
			// 중거리 이상 공격은 너무 가까우면 사용 불가 (최소 거리 = 최대 사거리의 30%)
			MinRange = Config.AttackDistance * 0.3f;
		}
		
		// 거리가 최소 거리 이상이고 최대 사거리 이하인 경우 사용 가능
		if (Distance >= MinRange && Distance <= Config.AttackDistance)
		{
			AvailableTypes.Add(AttackType);
		}
	}
	
	return AvailableTypes;
}

ELavaMonsterAttackType AAO_LavaMonster::SelectAttackTypeByRange() const
{
	if (!CurrentTarget.IsValid())
	{
		AO_LOG(LogKSJ, Warning, TEXT("SelectAttackTypeByRange: No current target"));
		return ELavaMonsterAttackType::MeleePunch; // 기본값
	}
	
	const float Distance = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
	TArray<ELavaMonsterAttackType> AvailableTypes = GetAvailableAttackTypesByRange(Distance);
	
	if (AvailableTypes.Num() == 0)
	{
		// 사용 가능한 공격이 없으면 기본값 (근접 공격)
		AO_LOG(LogKSJ, Log, TEXT("SelectAttackTypeByRange: No available attacks at distance %.1f, using default"), Distance);
		return ELavaMonsterAttackType::MeleePunch;
	}
	
	const int32 RandomIndex = FMath::RandRange(0, AvailableTypes.Num() - 1);
	const ELavaMonsterAttackType SelectedType = AvailableTypes[RandomIndex];
	
	AO_LOG(LogKSJ, Log, TEXT("SelectAttackTypeByRange: Selected attack type %d at distance %.1f (from %d available)"), 
		static_cast<int32>(SelectedType), Distance, AvailableTypes.Num());
	
	return SelectedType;
}

bool AAO_LavaMonster::IsTargetInAttackRange() const
{
	if (!CurrentTarget.IsValid())
	{
		return false;
	}

	// 최대 공격 사거리를 사용하여 공격 가능 여부 확인
	const float Distance = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
	const float MaxRange = GetMaxAttackRange();
	
	return Distance <= MaxRange;
}

float AAO_LavaMonster::GetMaxAttackRange() const
{
	float MaxRange = 0.f;
	
	for (const auto& Pair : AttackConfigs)
	{
		const FAO_LavaMonsterAttackConfig& Config = Pair.Value;
		const ELavaMonsterAttackType AttackType = Pair.Key;
		
		// 땅속 공격은 GroundStrikeRange 사용
		if (AttackType == ELavaMonsterAttackType::GroundStrike)
		{
			MaxRange = FMath::Max(MaxRange, Config.GroundStrikeRange);
		}
		else
		{
			// 일반 공격은 AttackDistance 사용
			MaxRange = FMath::Max(MaxRange, Config.AttackDistance);
		}
	}
	
	// 최소값은 기본 AttackRange (근접 공격 사거리)
	return FMath::Max(MaxRange, AttackRange);
}

FAO_LavaMonsterAttackConfig AAO_LavaMonster::GetAttackConfig(ELavaMonsterAttackType AttackType) const
{
	if (const FAO_LavaMonsterAttackConfig* Config = AttackConfigs.Find(AttackType))
	{
		return *Config;
	}

	// 기본값 반환
	return FAO_LavaMonsterAttackConfig();
}

void AAO_LavaMonster::ExecuteAttack(ELavaMonsterAttackType AttackType)
{
	if (bIsAttacking)
	{
		AO_LOG(LogKSJ, Warning, TEXT("ExecuteAttack: Already attacking"));
		return;
	}

	// 현재 공격 타입 저장
	CurrentAttackType = AttackType;

	// GAS Ability 실행 (AO_GA_LavaMonster_Attack)
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	bool bAbilityActivated = false;

	if (ASC)
	{
		// 1. 태그로 실행 시도
		FGameplayTagContainer TagContainer;
		TagContainer.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Combat.Attack")));
		if (ASC->TryActivateAbilitiesByTag(TagContainer, true))
		{
			bAbilityActivated = true;
		}
		// 2. 태그로 실패 시 클래스 상속 확인하여 실행 (백업)
		else
		{
			for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
			{
				if (Spec.Ability && Spec.Ability->IsA(UAO_GA_LavaMonster_Attack::StaticClass()))
				{
					if (ASC->TryActivateAbility(Spec.Handle))
					{
						bAbilityActivated = true;
						break;
					}
				}
			}
		}

		if (!bAbilityActivated)
		{
			AO_LOG(LogKSJ, Error, TEXT("ExecuteAttack: Failed to activate LavaMonster Attack Ability! Check Ability Tags or Constraints."));
		}
	}
	else
	{
		AO_LOG(LogKSJ, Error, TEXT("ExecuteAttack: No ASC found"));
	}
}

void AAO_LavaMonster::HandleStunBegin()
{
	Super::HandleStunBegin();

	// 공격 중이면 취소
	if (bIsAttacking)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->Montage_Stop(0.2f);
		}
		bIsAttacking = false;
	}

	// 기절 애니메이션 재생
	if (UAO_LavaMonster_AnimInstance* LavaMonsterAnimInstance = Cast<UAO_LavaMonster_AnimInstance>(GetMesh()->GetAnimInstance()))
	{
		LavaMonsterAnimInstance->PlayStunMontage();
	}
}

