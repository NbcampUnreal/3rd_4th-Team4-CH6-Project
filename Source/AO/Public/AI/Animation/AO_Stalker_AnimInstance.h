//KSJ : AO_Stalker_AnimInstance

#pragma once

#include "CoreMinimal.h"
#include "AI/Animation/AO_AIAnimInstance.h"
#include "AO_Stalker_AnimInstance.generated.h"

class AAO_Stalker;
class UAnimMontage;

/**
 * Stalker AI 애니메이션 인스턴스
 * 
 * 주요 기능:
 * - 속도 기반 Locomotion (로컬 좌표계)
 * - 천장 모드 상태 반영
 * - 기절 Montage 재생
 * - 공격 몽타주 선택
 */
UCLASS()
class AO_API UAO_Stalker_AnimInstance : public UAO_AIAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	// 공격 거리에 따른 몽타주 선택 (BP에서 호출)
	UFUNCTION(BlueprintCallable, Category = "AO|Animation|Stalker|Combat")
	UAnimMontage* GetAttackMontage() const;

	// 기절 몽타주 재생/중지
	UFUNCTION(BlueprintCallable, Category = "AO|Animation|Stalker")
	void PlayStunMontage();

	UFUNCTION(BlueprintCallable, Category = "AO|Animation|Stalker")
	void StopStunMontage(float BlendOutTime = 0.25f);

	// Getter 함수들 (블루프린트용)
	UFUNCTION(BlueprintPure, Category = "AO|Animation|Stalker")
	bool IsStunned() const { return bIsStunned; }

	UFUNCTION(BlueprintPure, Category = "AO|Animation|Stalker")
	bool IsInCeiling() const { return bInCeiling; }

	// 기절 몽타주 정보 Getter (Multicast용)
	UFUNCTION(BlueprintPure, Category = "AO|Animation|Stalker")
	UAnimMontage* GetStunMontage() const { return StunMontage; }

	UFUNCTION(BlueprintPure, Category = "AO|Animation|Stalker")
	float GetStunMontagePlayRate() const { return StunMontagePlayRate; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|Animation|Stalker")
	TObjectPtr<AAO_Stalker> StalkerCharacter;

	// === 상태 플래그 ===

	// 천장 모드
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|Animation|Stalker|State")
	bool bInCeiling = false;

	// 기절 상태
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|Animation|Stalker|State")
	bool bIsStunned = false;

	// === 이동 관련 ===

	// 로컬 좌표계 속도 (X: 전진/후진, Y: 좌우)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|Animation|Stalker|Movement")
	FVector LocalVelocity;

	// 전진/후진 속도 (양수: 전진, 음수: 후진) - BlendSpace 입력용
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|Animation|Stalker|Movement")
	float ForwardSpeed = 0.f;

	// 좌우 속도 (양수: 오른쪽, 음수: 왼쪽) - BlendSpace 입력용
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|Animation|Stalker|Movement")
	float RightSpeed = 0.f;

	// === 몽타주 설정 ===

	// 공격 몽타주 목록 (BP에서 할당)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Animation|Stalker|Montage")
	TArray<TObjectPtr<UAnimMontage>> AttackMontages;

	// 기절 몽타주 (에디터에서 할당)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Animation|Stalker|Montage")
	TObjectPtr<UAnimMontage> StunMontage;

	// 기절 몽타주 재생 속도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Animation|Stalker|Montage")
	float StunMontagePlayRate = 1.0f;
};

