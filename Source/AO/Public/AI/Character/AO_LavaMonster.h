// AO_LavaMonster.h

#pragma once

#include "CoreMinimal.h"
#include "AI/Base/AO_AggressiveAIBase.h"
#include "AO_LavaMonster.generated.h"

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

	// 공격 중인지
	UFUNCTION(BlueprintCallable, Category = "AO|AI|LavaMonster")
	bool IsAttacking() const { return bIsAttacking; }

	// 공격 상태 설정 (GAS에서 사용)
	void SetIsAttacking(bool bVal) { bIsAttacking = bVal; }

	// 현재 공격 타입 가져오기
	UFUNCTION(BlueprintCallable, Category = "AO|AI|LavaMonster")
	ELavaMonsterAttackType GetCurrentAttackType() const { return CurrentAttackType; }

	// 공격 범위 확인 오버라이드 (최대 공격 사거리 사용)
	virtual bool IsTargetInAttackRange() const override;

	// 최대 공격 사거리 가져오기
	UFUNCTION(BlueprintCallable, Category = "AO|AI|LavaMonster")
	float GetMaxAttackRange() const;

protected:
	virtual void BeginPlay() override;

	// 기절 처리 오버라이드
	virtual void HandleStunBegin() override;

protected:
	// 공격 타입별 설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|AI|LavaMonster|Attack")
	TMap<ELavaMonsterAttackType, FAO_LavaMonsterAttackConfig> AttackConfigs;

	// 공격 중 플래그
	UPROPERTY(BlueprintReadOnly, Category = "AO|AI|LavaMonster")
	bool bIsAttacking = false;

	// 현재 공격 타입
	UPROPERTY(BlueprintReadOnly, Category = "AO|AI|LavaMonster")
	ELavaMonsterAttackType CurrentAttackType = ELavaMonsterAttackType::MeleePunch;

	// 데미지 Effect 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|AI|LavaMonster|GAS")
	TSubclassOf<UGameplayEffect> DamageEffectClass;
};

