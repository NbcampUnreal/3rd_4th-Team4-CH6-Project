//KSJ : AO_Troll

#include "AI/Character/AO_Troll.h"
#include "AI/Component/AO_WeaponHolderComp.h"
#include "AI/Item/AO_TrollWeapon.h"
#include "AI/Animation/AO_Troll_AnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AI/GAS/Ability/AO_GA_Troll_Attack.h"

AAO_Troll::AAO_Troll()
{
	// 캡슐 콜리전 크기 설정 (대형 인간형 몬스터)
	GetCapsuleComponent()->SetCapsuleHalfHeight(250.f);
	GetCapsuleComponent()->SetCapsuleRadius(100.f);

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

	// 레벨 기반 머티리얼 적용
	ApplyLevelBasedMaterials();

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

	}
}

void AAO_Troll::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsAttacking = false;
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
	}

	// 무기 줍기 중이면 취소
	bIsPickingUpWeapon = false;

	// 서버에서 Multicast로 기절 애니메이션 재생 (모든 클라이언트에서 보이도록)
	if (HasAuthority())
	{
		if (UAO_Troll_AnimInstance* TrollAnimInstance = Cast<UAO_Troll_AnimInstance>(GetMesh()->GetAnimInstance()))
		{
			UAnimMontage* StunMontage = TrollAnimInstance->GetStunMontage();
			if (StunMontage)
			{
				Multicast_PlayStunMontage(StunMontage, TrollAnimInstance->GetStunMontagePlayRate());
			}
		}
	}
}

void AAO_Troll::HandleStunEnd()
{
	Super::HandleStunEnd();

	// 서버에서 Multicast로 기절 애니메이션 중지
	if (HasAuthority())
	{
		Multicast_StopStunMontage(0.25f);
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
	}
}

void AAO_Troll::ApplyLevelBasedMaterials()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// 현재 레벨 이름 가져오기
	FString CurrentLevelName = World->GetMapName();
	CurrentLevelName.RemoveFromStart(World->StreamingLevelsPrefix); // UEDPIE_ 접두사 제거

	// Ice 레벨 패턴 매칭
	bool bIsIceLevel = false;
	for (const FString& Pattern : IceLevelPatterns)
	{
		if (CurrentLevelName.Contains(Pattern))
		{
			bIsIceLevel = true;
			break;
		}
	}

	if (!bIsIceLevel)
	{
		return;
	}

	// Ice 머티리얼 적용
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!IsValid(MeshComp))
	{
		return;
	}

	if (IsValid(IceArmorMaterial))
	{
		MeshComp->SetMaterial(0, IceArmorMaterial); // Element 0: Armor
	}

	if (IsValid(IceBodyMaterial))
	{
		MeshComp->SetMaterial(1, IceBodyMaterial); // Element 1: Body
	}
}
