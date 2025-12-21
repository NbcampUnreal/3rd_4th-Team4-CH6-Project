// AO_Troll.cpp

#include "AI/Character/AO_Troll.h"
#include "AI/Component/AO_WeaponHolderComp.h"
#include "AI/Item/AO_TrollWeapon.h"
#include "AI/Animation/AO_Troll_AnimInstance.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AO_Log.h"
#include "AI/GAS/Ability/AO_GA_Troll_Attack.h"

AAO_Troll::AAO_Troll()
{
	// 무기 관리 컴포넌트 생성
	WeaponHolderComp = CreateDefaultSubobject<UAO_WeaponHolderComp>(TEXT("WeaponHolderComp"));

	// Troll 속도 설정
	RoamSpeed = 300.f;
	ChaseSpeed = 500.f;
	DefaultMovementSpeed = RoamSpeed;
	AlertMovementSpeed = ChaseSpeed;

	// 기본 공격 범위
	AttackRange = 250.f;
}

void AAO_Troll::BeginPlay()
{
	Super::BeginPlay();

	// 기본 공격 설정이 없으면 초기화
	if (AttackConfigs.Num() == 0)
	{
		// 횡 단일 공격
		FAO_TrollAttackConfig HorizontalSingle;
		HorizontalSingle.Damage = 25.f;
		HorizontalSingle.KnockbackStrength = 400.f;
		HorizontalSingle.AttackRadius = 200.f;
		HorizontalSingle.AttackDistance = 250.f;
		HorizontalSingle.bRequiresWeapon = true;
		AttackConfigs.Add(ETrollAttackType::HorizontalSingle, HorizontalSingle);

		// 횡 이중 공격
		FAO_TrollAttackConfig HorizontalDouble;
		HorizontalDouble.Damage = 20.f;
		HorizontalDouble.KnockbackStrength = 350.f;
		HorizontalDouble.AttackRadius = 200.f;
		HorizontalDouble.AttackDistance = 250.f;
		HorizontalDouble.bRequiresWeapon = true;
		AttackConfigs.Add(ETrollAttackType::HorizontalDouble, HorizontalDouble);

		// 종 내려찍기
		FAO_TrollAttackConfig VerticalSlam;
		VerticalSlam.Damage = 40.f;
		VerticalSlam.KnockbackStrength = 600.f;
		VerticalSlam.AttackRadius = 150.f;
		VerticalSlam.AttackDistance = 200.f;
		VerticalSlam.bRequiresWeapon = true;
		AttackConfigs.Add(ETrollAttackType::VerticalSlam, VerticalSlam);

		// 밟기 공격
		FAO_TrollAttackConfig Stomp;
		Stomp.Damage = 15.f;
		Stomp.KnockbackStrength = 300.f;
		Stomp.AttackRadius = 120.f;
		Stomp.AttackDistance = 150.f;
		Stomp.bRequiresWeapon = false;  // 무기 없이도 사용 가능
		AttackConfigs.Add(ETrollAttackType::Stomp, Stomp);
	}

	// 서버에서만 무기 스폰
	if (HasAuthority() && bSpawnWithWeapon && DefaultWeaponClass)
	{
		SpawnAndEquipWeapon();
	}
}

bool AAO_Troll::HasWeapon() const
{
	return WeaponHolderComp && WeaponHolderComp->HasWeapon();
}

ETrollAttackType AAO_Troll::SelectRandomAttackType() const
{
	TArray<ETrollAttackType> AvailableTypes = GetAvailableAttackTypes();
	
	if (AvailableTypes.Num() == 0)
	{
		// 사용 가능한 공격이 없으면 밟기로 기본값
		return ETrollAttackType::Stomp;
	}

	const int32 RandomIndex = FMath::RandRange(0, AvailableTypes.Num() - 1);
	return AvailableTypes[RandomIndex];
}

TArray<ETrollAttackType> AAO_Troll::GetAvailableAttackTypes() const
{
	TArray<ETrollAttackType> AvailableTypes;
	const bool bHasWeapon = HasWeapon();

	for (const auto& Pair : AttackConfigs)
	{
		const FAO_TrollAttackConfig& Config = Pair.Value;
		
		// 무기가 필요한 공격인데 무기가 없으면 제외
		if (Config.bRequiresWeapon && !bHasWeapon)
		{
			continue;
		}

		AvailableTypes.Add(Pair.Key);
	}

	return AvailableTypes;
}

FAO_TrollAttackConfig AAO_Troll::GetAttackConfig(ETrollAttackType AttackType) const
{
	if (const FAO_TrollAttackConfig* Config = AttackConfigs.Find(AttackType))
	{
		return *Config;
	}

	// 기본값 반환
	return FAO_TrollAttackConfig();
}

void AAO_Troll::ExecuteAttack(ETrollAttackType AttackType)
{
	if (bIsAttacking)
	{
		AO_LOG(LogKSJ, Warning, TEXT("ExecuteAttack: Already attacking"));
		return;
	}

	// GAS Ability 실행 (AO_GA_Troll_Attack)
	// Ability 내부에서 몽타주 재생, 데미지 처리, IsAttacking 상태 관리를 수행함.
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	bool bAbilityActivated = false;

	if (ASC)
	{
		// 1. 태그로 실행 시도 (가장 권장됨)
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
				if (Spec.Ability && Spec.Ability->IsA(UAO_GA_Troll_Attack::StaticClass()))
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
			// 실패 원인 로그 (쿨타임, 비용, 태그 등)
			AO_LOG(LogKSJ, Error, TEXT("ExecuteAttack: Failed to activate Troll Attack Ability! Check Ability Tags or Constraints."));
		}
	}
	else
	{
		AO_LOG(LogKSJ, Error, TEXT("ExecuteAttack: No ASC found"));
	}
}

void AAO_Troll::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsAttacking = false;

	if (bInterrupted)
	{
		AO_LOG(LogKSJ, Log, TEXT("%s: Attack interrupted"), *GetName());
	}
	else
	{
		AO_LOG(LogKSJ, Log, TEXT("%s: Attack completed"), *GetName());
	}
}

void AAO_Troll::HandleStunBegin()
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

	// 무기 드롭
	if (HasWeapon())
	{
		WeaponHolderComp->DropWeapon();
		AO_LOG(LogKSJ, Log, TEXT("%s: Dropped weapon due to stun"), *GetName());
	}

	// 무기 줍기 중이면 취소
	bIsPickingUpWeapon = false;

	// 기절 애니메이션 재생
	if (UAO_Troll_AnimInstance* TrollAnimInstance = Cast<UAO_Troll_AnimInstance>(GetMesh()->GetAnimInstance()))
	{
		TrollAnimInstance->PlayStunMontage();
	}
}

void AAO_Troll::HandleStunEnd()
{
	Super::HandleStunEnd();

	// 기절 애니메이션 중지
	if (UAO_Troll_AnimInstance* TrollAnimInstance = Cast<UAO_Troll_AnimInstance>(GetMesh()->GetAnimInstance()))
	{
		TrollAnimInstance->StopStunMontage(0.25f);
	}

	// 기절 해제 후 특별한 처리는 State Tree에서 담당
}

void AAO_Troll::SpawnAndEquipWeapon()
{
	if (!HasAuthority() || !DefaultWeaponClass || !WeaponHolderComp)
	{
		return;
	}

	// 이미 무기가 있으면 스킵
	if (WeaponHolderComp->HasWeapon())
	{
		return;
	}

	// 무기 스폰
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AAO_TrollWeapon* NewWeapon = GetWorld()->SpawnActor<AAO_TrollWeapon>(
		DefaultWeaponClass,
		GetActorLocation(),
		GetActorRotation(),
		SpawnParams
	);

	if (NewWeapon)
	{
		// 바로 장착
		WeaponHolderComp->PickupWeapon(NewWeapon);
		AO_LOG(LogKSJ, Log, TEXT("%s: Spawned and equipped weapon %s"), *GetName(), *NewWeapon->GetName());
	}
}
