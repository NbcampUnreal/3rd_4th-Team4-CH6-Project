//KSJ : AO_Bull_AnimInstance

#pragma once

#include "CoreMinimal.h"
#include "AI/Animation/AO_AIAnimInstance.h"
#include "AO_Bull_AnimInstance.generated.h"

class AAO_Bull;
class UAnimMontage;

/**
 * Bull AI 애니메이션 인스턴스
 * 
 * 주요 기능:
 * - 속도 기반 Locomotion
 * - 돌진(Charge) 상태 반영
 * - 기절 Montage 재생
 */
UCLASS()
class AO_API UAO_Bull_AnimInstance : public UAO_AIAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	// 기절 몽타주 재생/중지
	UFUNCTION(BlueprintCallable, Category = "AO|Animation|Bull")
	void PlayStunMontage();

	UFUNCTION(BlueprintCallable, Category = "AO|Animation|Bull")
	void StopStunMontage(float BlendOutTime = 0.25f);

	// Getter 함수들 (블루프린트용)
	UFUNCTION(BlueprintPure, Category = "AO|Animation|Bull")
	bool IsStunned() const { return bIsStunned; }

	UFUNCTION(BlueprintPure, Category = "AO|Animation|Bull")
	bool IsCharging() const { return bIsCharging; }

	// 기절 몽타주 정보 Getter (Multicast용)
	UFUNCTION(BlueprintPure, Category = "AO|Animation|Bull")
	UAnimMontage* GetStunMontage() const { return StunMontage; }

	UFUNCTION(BlueprintPure, Category = "AO|Animation|Bull")
	float GetStunMontagePlayRate() const { return StunMontagePlayRate; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|Animation|Bull")
	TObjectPtr<AAO_Bull> BullCharacter;

	// === 상태 플래그 ===

	// 돌진 중
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|Animation|Bull|State")
	bool bIsCharging = false;

	// 기절 상태
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|Animation|Bull|State")
	bool bIsStunned = false;

	// === 몽타주 설정 ===

	// 기절 몽타주 (에디터에서 할당)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Animation|Bull|Montage")
	TObjectPtr<UAnimMontage> StunMontage;

	// 기절 몽타주 재생 속도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Animation|Bull|Montage")
	float StunMontagePlayRate = 1.0f;
};

