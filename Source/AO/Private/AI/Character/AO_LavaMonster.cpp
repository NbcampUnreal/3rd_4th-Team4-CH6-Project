//KSJ : AO_LavaMonster

#include "AI/Character/AO_LavaMonster.h"
#include "AI/Animation/AO_LavaMonster_AnimInstance.h"
#include "Character/AO_PlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "AI/GAS/Ability/AO_GA_LavaMonster_Attack.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundAttenuation.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"

AAO_LavaMonster::AAO_LavaMonster()
{
	// 용암 몬스터 속도 설정
	RoamSpeed = 300.f;
	ChaseSpeed = 500.f;
	DefaultMovementSpeed = RoamSpeed;
	AlertMovementSpeed = ChaseSpeed;

	// 기본 공격 범위
	AttackRange = 250.f;

	// 이동 사운드 최대 속도 기본값 (ChaseSpeed와 동일)
	MaxSpeedForSound = ChaseSpeed;

	// Tick 활성화 (이동 사운드 파라미터 업데이트용)
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
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

	// 앰비언트 사운드 시작 (사운드와 Attenuation이 설정된 경우에만)
	StartAmbientSound();

	// 이동 사운드 시작 (항상 재생, 속도에 따라 볼륨 조절)
	StartMovementSound();
}

void AAO_LavaMonster::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 앰비언트 사운드 정지
	StopAmbientSound();

	// 이동 사운드 정지
	StopMovementSound();

	Super::EndPlay(EndPlayReason);
}

void AAO_LavaMonster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 이동 사운드 파라미터 업데이트
	UpdateMovementSoundParameters();
}

void AAO_LavaMonster::StartAmbientSound()
{
	// 이미 재생 중이면 무시
	if (IsAmbientSoundPlaying())
	{
		return;
	}

	// 사운드가 설정되지 않았으면 무시
	if (!LavaAmbientSound)
	{
		return;
	}

	// 오디오 컴포넌트 생성 및 설정
	AmbientAudioComponent = NewObject<UAudioComponent>(this, TEXT("LavaAmbientAudioComponent"));
	if (!ensure(AmbientAudioComponent))
	{
		return;
	}

	// 컴포넌트를 루트에 부착
	AmbientAudioComponent->SetupAttachment(GetRootComponent());
	AmbientAudioComponent->RegisterComponent();

	// 사운드 설정
	AmbientAudioComponent->SetSound(LavaAmbientSound);
	AmbientAudioComponent->SetVolumeMultiplier(AmbientSoundVolume);
	AmbientAudioComponent->SetPitchMultiplier(AmbientSoundPitch);

	// Attenuation 설정 (거리 감쇠)
	if (AmbientSoundAttenuation)
	{
		AmbientAudioComponent->AttenuationSettings = AmbientSoundAttenuation;
	}

	// 자동 파괴 비활성화 (수동 관리)
	AmbientAudioComponent->bAutoDestroy = false;

	// 루프 재생 설정은 MetaSound 내부에서 처리됨 (Trigger Repeat로 무한 반복)
	// 여기서는 재생만 시작
	AmbientAudioComponent->Play();
}

void AAO_LavaMonster::StopAmbientSound()
{
	if (AmbientAudioComponent)
	{
		if (AmbientAudioComponent->IsPlaying())
		{
			AmbientAudioComponent->Stop();
		}

		AmbientAudioComponent->DestroyComponent();
		AmbientAudioComponent = nullptr;
	}
}

bool AAO_LavaMonster::IsAmbientSoundPlaying() const
{
	return AmbientAudioComponent && AmbientAudioComponent->IsPlaying();
}

void AAO_LavaMonster::StartMovementSound()
{
	// 이미 재생 중이면 무시
	if (IsMovementSoundPlaying())
	{
		return;
	}

	// 사운드가 설정되지 않았으면 무시
	if (!MovementSound)
	{
		return;
	}

	// 오디오 컴포넌트 생성 및 설정
	MovementAudioComponent = NewObject<UAudioComponent>(this, TEXT("LavaMovementAudioComponent"));
	if (!ensure(MovementAudioComponent))
	{
		return;
	}

	// 컴포넌트를 루트에 부착
	MovementAudioComponent->SetupAttachment(GetRootComponent());
	MovementAudioComponent->RegisterComponent();

	// 사운드 설정
	MovementAudioComponent->SetSound(MovementSound);
	MovementAudioComponent->SetVolumeMultiplier(MovementSoundVolume);
	MovementAudioComponent->SetPitchMultiplier(MovementSoundPitch);

	// Attenuation 설정 (거리 감쇠)
	if (MovementSoundAttenuation)
	{
		MovementAudioComponent->AttenuationSettings = MovementSoundAttenuation;
	}

	// 자동 파괴 비활성화 (수동 관리)
	MovementAudioComponent->bAutoDestroy = false;

	// 초기 MovementSpeed 파라미터 설정 (0으로 시작)
	MovementAudioComponent->SetFloatParameter(MovementSpeedParamName, 0.f);

	// 재생 시작
	MovementAudioComponent->Play();
}

void AAO_LavaMonster::StopMovementSound()
{
	if (MovementAudioComponent)
	{
		if (MovementAudioComponent->IsPlaying())
		{
			MovementAudioComponent->Stop();
		}

		MovementAudioComponent->DestroyComponent();
		MovementAudioComponent = nullptr;
	}

	PreviousNormalizedSpeed = 0.f;
}

bool AAO_LavaMonster::IsMovementSoundPlaying() const
{
	return MovementAudioComponent && MovementAudioComponent->IsPlaying();
}

float AAO_LavaMonster::GetNormalizedMovementSpeed() const
{
	// 현재 속도 가져오기
	const float CurrentSpeed = GetVelocity().Size();

	// 0~1 범위로 정규화 (MaxSpeedForSound 기준)
	if (MaxSpeedForSound <= 0.f)
	{
		return 0.f;
	}

	return FMath::Clamp(CurrentSpeed / MaxSpeedForSound, 0.f, 1.f);
}

void AAO_LavaMonster::UpdateMovementSoundParameters()
{
	// 이동 사운드가 재생 중이 아니면 무시
	if (!IsMovementSoundPlaying())
	{
		return;
	}

	// 현재 정규화된 속도 가져오기
	const float TargetSpeed = GetNormalizedMovementSpeed();

	// 스무딩 적용 (급격한 변화 방지)
	const float SmoothedSpeed = FMath::FInterpTo(PreviousNormalizedSpeed, TargetSpeed, GetWorld()->GetDeltaSeconds(), 1.f / SpeedSmoothingFactor);
	PreviousNormalizedSpeed = SmoothedSpeed;

	// MetaSound 파라미터 업데이트
	MovementAudioComponent->SetFloatParameter(MovementSpeedParamName, SmoothedSpeed);
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
		return ELavaMonsterAttackType::MeleePunch; // 기본값
	}
	
	const float Distance = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
	TArray<ELavaMonsterAttackType> AvailableTypes = GetAvailableAttackTypesByRange(Distance);
	
	if (AvailableTypes.Num() == 0)
	{
		// 사용 가능한 공격이 없으면 기본값 (근접 공격)
		return ELavaMonsterAttackType::MeleePunch;
	}
	
	const int32 RandomIndex = FMath::RandRange(0, AvailableTypes.Num() - 1);
	return AvailableTypes[RandomIndex];
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

	// 서버에서 Multicast로 기절 애니메이션 재생 (모든 클라이언트에서 보이도록)
	if (HasAuthority())
	{
		if (UAO_LavaMonster_AnimInstance* LavaMonsterAnimInstance = Cast<UAO_LavaMonster_AnimInstance>(GetMesh()->GetAnimInstance()))
		{
			UAnimMontage* StunMontage = LavaMonsterAnimInstance->GetStunMontage();
			if (StunMontage)
			{
				Multicast_PlayStunMontage(StunMontage, LavaMonsterAnimInstance->GetStunMontagePlayRate());
			}
		}
	}
}

void AAO_LavaMonster::Multicast_SpawnGroundStrikeVFX_Implementation(const FVector& Location, float Radius, float Duration)
{
	// VFX 에셋이 설정되지 않았으면 리턴
	if (!GroundStrikeVFX)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// 위치를 약간 위로 올림 (바닥에 묻히지 않게, Z-fighting 방지)
	FVector SpawnLocation = Location;
	SpawnLocation.Z += 1.f;

	// 나이아가라 시스템 스폰
	UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		World,
		GroundStrikeVFX,
		SpawnLocation,
		FRotator::ZeroRotator,
		FVector::OneVector,
		true,  // bAutoDestroy - VFX 종료 후 자동 삭제
		true,  // bAutoActivate - 즉시 활성화
		ENCPoolMethod::None,
		true   // bPreCullCheck
	);

	if (NiagaraComp)
	{
		// User Parameter 설정 - 나이아가라 시스템에서 정의한 파라미터에 값 전달
		NiagaraComp->SetFloatParameter(FName("StrikeRadius"), Radius);
		NiagaraComp->SetFloatParameter(FName("WarningDuration"), Duration);
	}
}

void AAO_LavaMonster::Multicast_SpawnEruptionVFX_Implementation(const FVector& Location)
{
	// VFX 에셋이 설정되지 않았으면 리턴
	if (!EruptionVFX)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// 분출 VFX 스폰 (Cascade)
	UGameplayStatics::SpawnEmitterAtLocation(
		World,
		EruptionVFX,
		Location + FVector(0.f, 0.f, 1.f),  // 바닥에서 약간 위
		FRotator::ZeroRotator,
		FVector(1.f),  // 스케일
		true,  // bAutoDestroy
		EPSCPoolMethod::None,
		true   // bAutoActivate
	);
}

