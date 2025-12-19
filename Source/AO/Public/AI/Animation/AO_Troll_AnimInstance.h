// AO_Troll_AnimInstance.h

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "AO_Troll_AnimInstance.generated.h"

class AAO_Troll;
class UAnimMontage;

/**
 * Troll AI 애니메이션 인스턴스
 * 
 * 주요 기능:
 * - 속도 기반 Locomotion (Idle/Walk 블렌딩)
 * - 기절 Montage 재생
 * - 무기 줍기 Montage 재생
 * - 공격 상태 반영
 */
UCLASS()
class AO_API UAO_Troll_AnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UAO_Troll_AnimInstance();

	// UAnimInstance 오버라이드
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	// 몽타주 재생 함수들
	UFUNCTION(BlueprintCallable, Category = "AO|Animation|Troll")
	void PlayStunMontage();

	UFUNCTION(BlueprintCallable, Category = "AO|Animation|Troll")
	void StopStunMontage(float BlendOutTime = 0.25f);

	UFUNCTION(BlueprintCallable, Category = "AO|Animation|Troll")
	void PlayPickupWeaponMontage();

	// Getter 함수들 (블루프린트용)
	UFUNCTION(BlueprintPure, Category = "AO|Animation|Troll")
	float GetSpeed() const { return Speed; }

	UFUNCTION(BlueprintPure, Category = "AO|Animation|Troll")
	bool IsMoving() const { return bIsMoving; }

	UFUNCTION(BlueprintPure, Category = "AO|Animation|Troll")
	bool IsStunned() const { return bIsStunned; }

	UFUNCTION(BlueprintPure, Category = "AO|Animation|Troll")
	bool IsAttacking() const { return bIsAttacking; }

	UFUNCTION(BlueprintPure, Category = "AO|Animation|Troll")
	bool HasWeapon() const { return bHasWeapon; }

	UFUNCTION(BlueprintPure, Category = "AO|Animation|Troll")
	bool IsPickingUpWeapon() const { return bIsPickingUpWeapon; }

protected:
	// 캐릭터 참조 업데이트
	void UpdateCharacterReference();

	// 애니메이션 상태 업데이트
	void UpdateAnimationProperties();

protected:
	// 소유 Troll 캐릭터 참조
	UPROPERTY(BlueprintReadOnly, Category = "AO|Animation|Troll")
	TWeakObjectPtr<AAO_Troll> OwningTroll;

	// === Locomotion 변수 ===

	// 현재 이동 속도 (Blend Space 입력값)
	UPROPERTY(BlueprintReadOnly, Category = "AO|Animation|Troll|Locomotion")
	float Speed = 0.f;

	// 이동 중인지 여부
	UPROPERTY(BlueprintReadOnly, Category = "AO|Animation|Troll|Locomotion")
	bool bIsMoving = false;

	// === 상태 플래그 ===

	// 기절 상태
	UPROPERTY(BlueprintReadOnly, Category = "AO|Animation|Troll|State")
	bool bIsStunned = false;

	// 공격 중
	UPROPERTY(BlueprintReadOnly, Category = "AO|Animation|Troll|State")
	bool bIsAttacking = false;

	// 무기 소지
	UPROPERTY(BlueprintReadOnly, Category = "AO|Animation|Troll|State")
	bool bHasWeapon = false;

	// 무기 줍는 중
	UPROPERTY(BlueprintReadOnly, Category = "AO|Animation|Troll|State")
	bool bIsPickingUpWeapon = false;

	// === 몽타주 설정 ===

	// 기절 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Animation|Troll|Montage")
	TObjectPtr<UAnimMontage> StunMontage;

	// 무기 줍기 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Animation|Troll|Montage")
	TObjectPtr<UAnimMontage> PickupWeaponMontage;

	// 기절 몽타주 재생 속도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Animation|Troll|Montage")
	float StunMontagePlayRate = 1.0f;

	// 무기 줍기 몽타주 재생 속도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Animation|Troll|Montage")
	float PickupWeaponMontagePlayRate = 1.0f;
};
