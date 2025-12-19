// AO_Crab_AnimInstance.h

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "AO_Crab_AnimInstance.generated.h"

class AAO_Crab;
class UAnimMontage;

/**
 * Crab AI 애니메이션 인스턴스
 * 
 * 주요 기능:
 * - 속도 기반 Locomotion (Idle/Walk/Run 블렌딩)
 * - 기절 Montage 재생
 * - 아이템 운반 상태 반영
 */
UCLASS()
class AO_API UAO_Crab_AnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UAO_Crab_AnimInstance();

	// UAnimInstance 오버라이드
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	// 기절 Montage 제어
	UFUNCTION(BlueprintCallable, Category = "AO|Animation|Crab")
	void PlayStunMontage();

	UFUNCTION(BlueprintCallable, Category = "AO|Animation|Crab")
	void StopStunMontage(float BlendOutTime = 0.25f);

	// Getter 함수들 (블루프린트용)
	UFUNCTION(BlueprintPure, Category = "AO|Animation|Crab")
	float GetSpeed() const { return Speed; }

	UFUNCTION(BlueprintPure, Category = "AO|Animation|Crab")
	bool IsMoving() const { return bIsMoving; }

	UFUNCTION(BlueprintPure, Category = "AO|Animation|Crab")
	bool IsStunned() const { return bIsStunned; }

	UFUNCTION(BlueprintPure, Category = "AO|Animation|Crab")
	bool IsCarryingItem() const { return bIsCarryingItem; }

	UFUNCTION(BlueprintPure, Category = "AO|Animation|Crab")
	bool IsFleeing() const { return bIsFleeing; }

protected:
	// 캐릭터 참조 업데이트
	void UpdateCharacterReference();

	// 애니메이션 상태 업데이트
	void UpdateAnimationProperties();

protected:
	// 소유 Crab 캐릭터 참조
	UPROPERTY(BlueprintReadOnly, Category = "AO|Animation|Crab")
	TWeakObjectPtr<AAO_Crab> OwningCrab;

	// === 블렌드 스페이스용 변수 ===

	// 현재 이동 속도 (Blend Space 입력값)
	UPROPERTY(BlueprintReadOnly, Category = "AO|Animation|Crab|Locomotion")
	float Speed = 0.f;

	// 이동 중인지 여부
	UPROPERTY(BlueprintReadOnly, Category = "AO|Animation|Crab|Locomotion")
	bool bIsMoving = false;

	// === 상태 변수 ===

	// 기절 상태
	UPROPERTY(BlueprintReadOnly, Category = "AO|Animation|Crab|State")
	bool bIsStunned = false;

	// 아이템 운반 중
	UPROPERTY(BlueprintReadOnly, Category = "AO|Animation|Crab|State")
	bool bIsCarryingItem = false;

	// 도망 중
	UPROPERTY(BlueprintReadOnly, Category = "AO|Animation|Crab|State")
	bool bIsFleeing = false;

	// === Montage 설정 ===

	// 기절 Montage (에디터에서 설정)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Animation|Crab|Montage")
	TObjectPtr<UAnimMontage> StunMontage;

	// 기절 Montage 재생 속도 (빠르게 반복하려면 높게 설정)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Animation|Crab|Montage")
	float StunMontagePlayRate = 2.0f;

	// === 속도 임계값 (블렌드 스페이스 참고용) ===

	// 이동 중으로 판단하는 최소 속도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Animation|Crab|Threshold")
	float MovingThreshold = 10.f;
};
