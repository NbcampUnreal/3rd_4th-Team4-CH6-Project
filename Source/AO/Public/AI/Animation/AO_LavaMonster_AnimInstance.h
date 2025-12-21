// AO_LavaMonster_AnimInstance.h

#pragma once

#include "CoreMinimal.h"
#include "AI/Animation/AO_AIAnimInstance.h"
#include "AO_LavaMonster_AnimInstance.generated.h"

class AAO_LavaMonster;
class UAnimMontage;

/**
 * 용암 몬스터 AI 애니메이션 인스턴스
 * 
 * 주요 기능:
 * - 속도 기반 Locomotion (Idle/Walk 블렌딩)
 * - Idle 모션 4가지 랜덤 재생
 * - 기절 Montage 재생
 * - 공격 상태 반영
 */
UCLASS()
class AO_API UAO_LavaMonster_AnimInstance : public UAO_AIAnimInstance
{
	GENERATED_BODY()

public:
	UAO_LavaMonster_AnimInstance();

	// UAnimInstance 오버라이드
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	// 몽타주 재생 함수들
	UFUNCTION(BlueprintCallable, Category = "AO|Animation|LavaMonster")
	void PlayStunMontage();

	UFUNCTION(BlueprintCallable, Category = "AO|Animation|LavaMonster")
	void StopStunMontage(float BlendOutTime = 0.25f);

	// Idle 랜덤 재생
	UFUNCTION(BlueprintCallable, Category = "AO|Animation|LavaMonster")
	void PlayRandomIdleMontage();

	// Getter 함수들 (블루프린트용)
	UFUNCTION(BlueprintPure, Category = "AO|Animation|LavaMonster")
	bool IsStunned() const { return bIsStunned; }

	UFUNCTION(BlueprintPure, Category = "AO|Animation|LavaMonster")
	bool IsAttacking() const { return bIsAttacking; }

protected:
	// 캐릭터 참조 업데이트
	void UpdateCharacterReference();

	// 애니메이션 상태 업데이트
	void UpdateAnimationProperties();

	// Idle 랜덤 선택 및 재생
	void SelectAndPlayIdle();

protected:
	// 소유 LavaMonster 캐릭터 참조
	UPROPERTY(BlueprintReadOnly, Category = "AO|Animation|LavaMonster")
	TWeakObjectPtr<AAO_LavaMonster> OwningLavaMonster;

	// === 상태 플래그 ===

	// 기절 상태
	UPROPERTY(BlueprintReadOnly, Category = "AO|Animation|LavaMonster|State")
	bool bIsStunned = false;

	// 공격 중
	UPROPERTY(BlueprintReadOnly, Category = "AO|Animation|LavaMonster|State")
	bool bIsAttacking = false;

	// === 몽타주 설정 ===

	// 기절 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Animation|LavaMonster|Montage")
	TObjectPtr<UAnimMontage> StunMontage;

	// Idle 몽타주 4가지 (랜덤 재생)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Animation|LavaMonster|Montage")
	TArray<TObjectPtr<UAnimMontage>> IdleMontages;

	// 기절 몽타주 재생 속도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Animation|LavaMonster|Montage")
	float StunMontagePlayRate = 1.0f;

	// Idle 몽타주 재생 속도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Animation|LavaMonster|Montage")
	float IdleMontagePlayRate = 1.0f;

	// Idle 랜덤 재생 간격
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Animation|LavaMonster|Montage")
	float IdleRandomInterval = 5.f;

	// Idle 랜덤 재생 간격 랜덤 편차
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Animation|LavaMonster|Montage")
	float IdleRandomIntervalDeviation = 2.f;

private:
	// Idle 재생 타이머 핸들
	FTimerHandle IdleRandomTimerHandle;

	// 현재 재생 중인 Idle 몽타주
	UPROPERTY()
	TObjectPtr<UAnimMontage> CurrentIdleMontage;
};

