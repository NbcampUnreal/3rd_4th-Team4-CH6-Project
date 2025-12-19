// AO_Troll.h

#pragma once

#include "CoreMinimal.h"
#include "AI/Base/AO_AggressiveAIBase.h"
#include "AO_Troll.generated.h"

class UAO_WeaponHolderComp;
class AAO_TrollWeapon;

/**
 * Troll 공격 타입 열거형
 * 
 * - HorizontalSingle: 횡으로 한번 휘두르기
 * - HorizontalDouble: 횡으로 두번 휘두르기
 * - VerticalSlam: 종으로 내려찍기
 * - Stomp: 발로 밟기 (무기 없이도 사용 가능)
 */
UENUM(BlueprintType)
enum class ETrollAttackType : uint8
{
	HorizontalSingle	UMETA(DisplayName = "Horizontal Single"),
	HorizontalDouble	UMETA(DisplayName = "Horizontal Double"),
	VerticalSlam		UMETA(DisplayName = "Vertical Slam"),
	Stomp				UMETA(DisplayName = "Stomp")
};

/**
 * Troll 공격 설정 구조체
 */
USTRUCT(BlueprintType)
struct FAO_TrollAttackConfig
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

	// 공격 범위
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	float AttackRadius = 150.f;

	// 공격 거리 (전방)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	float AttackDistance = 200.f;

	// 무기 필요 여부
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	bool bRequiresWeapon = true;
};

/**
 * Troll AI 캐릭터
 * 
 * 특징:
 * - 무기를 들고 공격 (4종류 공격)
 * - 기절 시 무기 드롭
 * - 무기 없으면 밟기 공격만 가능
 * - 무기가 없을 때 플레이어보다 무기 줍기 우선
 * 
 * 속도:
 * - 기본: 300
 * - 추격: 500
 */
UCLASS()
class AO_API AAO_Troll : public AAO_AggressiveAIBase
{
	GENERATED_BODY()

public:
	AAO_Troll();

	// 무기 컴포넌트 접근
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Troll")
	UAO_WeaponHolderComp* GetWeaponHolderComponent() const { return WeaponHolderComp; }

	// 무기 소지 여부
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Troll")
	bool HasWeapon() const;

	// 랜덤 공격 타입 선택
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Troll")
	ETrollAttackType SelectRandomAttackType() const;

	// 현재 사용 가능한 공격 타입 목록
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Troll")
	TArray<ETrollAttackType> GetAvailableAttackTypes() const;

	// 공격 설정 가져오기
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Troll")
	FAO_TrollAttackConfig GetAttackConfig(ETrollAttackType AttackType) const;

	// 공격 실행
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Troll")
	void ExecuteAttack(ETrollAttackType AttackType);

	// 공격 중인지
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Troll")
	bool IsAttacking() const { return bIsAttacking; }

	// 공격 상태 설정 (GAS에서 사용)
	void SetIsAttacking(bool bVal) { bIsAttacking = bVal; }

	// 무기 줍기 중인지
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Troll")
	bool IsPickingUpWeapon() const { return bIsPickingUpWeapon; }

	// 무기 줍기 모드 설정
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Troll")
	void SetPickingUpWeapon(bool bPicking) { bIsPickingUpWeapon = bPicking; }

	// 무기 스폰 및 장착
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Troll")
	void SpawnAndEquipWeapon();

protected:
	virtual void BeginPlay() override;

	// 기절 처리 오버라이드
	virtual void HandleStunBegin() override;
	virtual void HandleStunEnd() override;

	// 공격 몽타주 종료 콜백
	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

protected:
	// 무기 관리 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|AI|Troll")
	TObjectPtr<UAO_WeaponHolderComp> WeaponHolderComp;

	// 공격 타입별 설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|AI|Troll|Attack")
	TMap<ETrollAttackType, FAO_TrollAttackConfig> AttackConfigs;

	// 공격 중 플래그
	UPROPERTY(BlueprintReadOnly, Category = "AO|AI|Troll")
	bool bIsAttacking = false;

	// 무기 줍는 중 플래그
	UPROPERTY(BlueprintReadOnly, Category = "AO|AI|Troll")
	bool bIsPickingUpWeapon = false;

	// 현재 공격 타입
	UPROPERTY(BlueprintReadOnly, Category = "AO|AI|Troll")
	ETrollAttackType CurrentAttackType = ETrollAttackType::HorizontalSingle;

	// 데미지 Effect 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|AI|Troll|GAS")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	// 스폰 시 생성할 무기 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AO|AI|Troll|Weapon")
	TSubclassOf<AAO_TrollWeapon> DefaultWeaponClass;

	// 스폰 시 무기를 자동으로 장착할지 여부
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AO|AI|Troll|Weapon")
	bool bSpawnWithWeapon = true;
};
