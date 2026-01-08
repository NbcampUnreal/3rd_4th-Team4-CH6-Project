//KSJ : AO_LavaMonster

#pragma once

#include "CoreMinimal.h"
#include "AI/Base/AO_AggressiveAIBase.h"
#include "AO_LavaMonster.generated.h"

class UAudioComponent;
class USoundAttenuation;
class UNiagaraSystem;
class UParticleSystem;

/**
 * 용암 몬스터 공격 타입 열거형
 * 
 * - MeleePunch: 근접 주먹 지르기
 * - MeleeUppercut: 근접 어퍼컷
 * - WhipMidRange: 중거리 채찍 공격
 * - ExtendArm: 중~원거리 팔 늘리기 공격
 * - GroundStrike: 땅속 팔 범위 공격 (범위 내 모든 플레이어 타겟팅)
 */
UENUM(BlueprintType)
enum class ELavaMonsterAttackType : uint8
{
	MeleePunch		UMETA(DisplayName = "Melee Punch"),
	MeleeUppercut	UMETA(DisplayName = "Melee Uppercut"),
	WhipMidRange	UMETA(DisplayName = "Whip Mid Range"),
	ExtendArm		UMETA(DisplayName = "Extend Arm"),
	GroundStrike	UMETA(DisplayName = "Ground Strike")
};

/**
 * 용암 몬스터 공격 설정 구조체
 */
USTRUCT(BlueprintType)
struct FAO_LavaMonsterAttackConfig
{
	GENERATED_BODY()

	// 공격 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	TObjectPtr<UAnimMontage> AttackMontage = nullptr;

	// 데미지
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	float Damage = 30.f;

	// 넉백 강도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	float KnockbackStrength = 500.f;

	// 공격 범위 (SphereTrace 반경)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	float AttackRadius = 150.f;

	// 공격 거리 (전방)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	float AttackDistance = 200.f;

	// 땅속 공격 전용: 범위 공격 반경
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|GroundStrike")
	float GroundStrikeRange = 1000.f;

	// 땅속 공격 전용: 전조 현상 지속 시간
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|GroundStrike")
	float WarningDuration = 2.f;
};

/**
 * 용암 몬스터 AI 캐릭터
 * 
 * 특징:
 * - 5가지 공격 타입 (근접 2, 중거리 1, 중~원거리 1, 범위 공격 1)
 * - 땅속 범위 공격은 범위 내 모든 플레이어 타겟팅
 * - Idle 모션 4가지 랜덤 재생
 * 
 * 속도:
 * - 기본: 300
 * - 추격: 500
 */
UCLASS()
class AO_API AAO_LavaMonster : public AAO_AggressiveAIBase
{
	GENERATED_BODY()

public:
	AAO_LavaMonster();

	// 랜덤 공격 타입 선택
	UFUNCTION(BlueprintCallable, Category = "AO|AI|LavaMonster")
	ELavaMonsterAttackType SelectRandomAttackType() const;

	// 현재 사용 가능한 공격 타입 목록
	UFUNCTION(BlueprintCallable, Category = "AO|AI|LavaMonster")
	TArray<ELavaMonsterAttackType> GetAvailableAttackTypes() const;

	// 사거리 기반 공격 타입 선택 (현재 타겟까지의 거리를 고려)
	UFUNCTION(BlueprintCallable, Category = "AO|AI|LavaMonster")
	ELavaMonsterAttackType SelectAttackTypeByRange() const;

	// 특정 거리에서 사용 가능한 공격 타입 목록
	UFUNCTION(BlueprintCallable, Category = "AO|AI|LavaMonster")
	TArray<ELavaMonsterAttackType> GetAvailableAttackTypesByRange(float Distance) const;

	// 공격 설정 가져오기
	UFUNCTION(BlueprintCallable, Category = "AO|AI|LavaMonster")
	FAO_LavaMonsterAttackConfig GetAttackConfig(ELavaMonsterAttackType AttackType) const;

	// 공격 실행
	UFUNCTION(BlueprintCallable, Category = "AO|AI|LavaMonster")
	void ExecuteAttack(ELavaMonsterAttackType AttackType);

	// 현재 공격 타입 가져오기
	UFUNCTION(BlueprintCallable, Category = "AO|AI|LavaMonster")
	ELavaMonsterAttackType GetCurrentAttackType() const { return CurrentAttackType; }

	// 공격 범위 확인 오버라이드 (최대 공격 사거리 사용)
	virtual bool IsTargetInAttackRange() const override;

	// 최대 공격 사거리 가져오기
	UFUNCTION(BlueprintCallable, Category = "AO|AI|LavaMonster")
	float GetMaxAttackRange() const;

	// ===== VFX 멀티캐스트 (모든 클라이언트에서 스폰) =====
	
	// 땅속 공격 전조 VFX 스폰 (나이아가라)
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SpawnGroundStrikeVFX(const FVector& Location, float Radius, float Duration);

	// 땅속 공격 분출 VFX 스폰 (Cascade)
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SpawnEruptionVFX(const FVector& Location);

public:
	// ===== 앰비언트 사운드 제어 =====
	UFUNCTION(BlueprintCallable, Category = "AO|AI|LavaMonster|Audio")
	void StartAmbientSound();

	UFUNCTION(BlueprintCallable, Category = "AO|AI|LavaMonster|Audio")
	void StopAmbientSound();

	UFUNCTION(BlueprintCallable, Category = "AO|AI|LavaMonster|Audio")
	bool IsAmbientSoundPlaying() const;

	// ===== 이동 사운드 제어 =====
	UFUNCTION(BlueprintCallable, Category = "AO|AI|LavaMonster|Audio")
	void StartMovementSound();

	UFUNCTION(BlueprintCallable, Category = "AO|AI|LavaMonster|Audio")
	void StopMovementSound();

	UFUNCTION(BlueprintCallable, Category = "AO|AI|LavaMonster|Audio")
	bool IsMovementSoundPlaying() const;

	// 현재 이동 속도 비율 (0~1) 가져오기
	UFUNCTION(BlueprintCallable, Category = "AO|AI|LavaMonster|Audio")
	float GetNormalizedMovementSpeed() const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;

	// 기절 처리 오버라이드
	virtual void HandleStunBegin() override;

	// 이동 사운드 파라미터 업데이트 (Tick에서 호출)
	void UpdateMovementSoundParameters();

protected:
	// ===== 오디오 설정 =====
	
	// 용암 앰비언트 사운드 (MetaSound Source 할당)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|AI|LavaMonster|Audio")
	TObjectPtr<USoundBase> LavaAmbientSound = nullptr;

	// 사운드 감쇠 설정 (거리에 따른 볼륨 감소, 3D 공간화)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|AI|LavaMonster|Audio")
	TObjectPtr<USoundAttenuation> AmbientSoundAttenuation = nullptr;

	// 앰비언트 사운드 볼륨 (0.0 ~ 2.0)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|AI|LavaMonster|Audio", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float AmbientSoundVolume = 1.0f;

	// 앰비언트 사운드 피치 (0.5 ~ 2.0)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|AI|LavaMonster|Audio", meta = (ClampMin = "0.5", ClampMax = "2.0"))
	float AmbientSoundPitch = 1.0f;

	// 앰비언트 오디오 컴포넌트 (런타임에 생성)
	UPROPERTY()
	TObjectPtr<UAudioComponent> AmbientAudioComponent = nullptr;

	// ===== 이동 사운드 설정 =====
	
	// 이동 사운드 (MetaSound Source - MovementSpeed 파라미터 필요)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|AI|LavaMonster|Audio|Movement")
	TObjectPtr<USoundBase> MovementSound = nullptr;

	// 이동 사운드 감쇠 설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|AI|LavaMonster|Audio|Movement")
	TObjectPtr<USoundAttenuation> MovementSoundAttenuation = nullptr;

	// 이동 사운드 기본 볼륨 (0.0 ~ 2.0)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|AI|LavaMonster|Audio|Movement", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float MovementSoundVolume = 1.0f;

	// 이동 사운드 기본 피치 (0.5 ~ 2.0)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|AI|LavaMonster|Audio|Movement", meta = (ClampMin = "0.5", ClampMax = "2.0"))
	float MovementSoundPitch = 1.0f;

	// 이동 속도 정규화를 위한 최대 속도 (ChaseSpeed 사용)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|AI|LavaMonster|Audio|Movement")
	float MaxSpeedForSound = 500.f;

	// MetaSound 파라미터 이름 (MovementSpeed)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|AI|LavaMonster|Audio|Movement")
	FName MovementSpeedParamName = FName("MovementSpeed");

	// 이동 오디오 컴포넌트 (런타임에 생성)
	UPROPERTY()
	TObjectPtr<UAudioComponent> MovementAudioComponent = nullptr;

	// 이전 프레임 이동 속도 (스무딩용)
	float PreviousNormalizedSpeed = 0.f;

	// 속도 스무딩 계수 (급격한 변화 방지)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|AI|LavaMonster|Audio|Movement", meta = (ClampMin = "0.01", ClampMax = "1.0"))
	float SpeedSmoothingFactor = 0.1f;

	// ===== 공격 설정 =====
	
	// 공격 타입별 설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|AI|LavaMonster|Attack")
	TMap<ELavaMonsterAttackType, FAO_LavaMonsterAttackConfig> AttackConfigs;

	// 현재 공격 타입
	UPROPERTY(BlueprintReadOnly, Category = "AO|AI|LavaMonster")
	ELavaMonsterAttackType CurrentAttackType = ELavaMonsterAttackType::MeleePunch;

	// 데미지 Effect 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|AI|LavaMonster|GAS")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	// ===== VFX 설정 (멀티캐스트용) =====
	
	// 땅속 공격 전조 VFX (나이아가라 시스템)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|AI|LavaMonster|VFX")
	TObjectPtr<UNiagaraSystem> GroundStrikeVFX = nullptr;

	// 땅속 공격 분출 VFX (Cascade 파티클)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|AI|LavaMonster|VFX")
	TObjectPtr<UParticleSystem> EruptionVFX = nullptr;
};

