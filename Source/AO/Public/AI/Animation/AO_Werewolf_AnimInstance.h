//KSJ : AO_Werewolf_AnimInstance

#pragma once

#include "CoreMinimal.h"
#include "AI/Animation/AO_AIAnimInstance.h"
#include "AO_Werewolf_AnimInstance.generated.h"

class AAO_Werewolf;
class UAnimMontage;

/**
 * Werewolf AI 애니메이션 인스턴스
 * 
 * 주요 기능:
 * - 속도 기반 Locomotion
 * - 기절 Montage 재생
 */
UCLASS()
class AO_API UAO_Werewolf_AnimInstance : public UAO_AIAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	// 기절 몽타주 재생/중지
	UFUNCTION(BlueprintCallable, Category = "AO|Animation|Werewolf")
	void PlayStunMontage();

	UFUNCTION(BlueprintCallable, Category = "AO|Animation|Werewolf")
	void StopStunMontage(float BlendOutTime = 0.25f);

	// Getter 함수들 (블루프린트용)
	UFUNCTION(BlueprintPure, Category = "AO|Animation|Werewolf")
	bool IsStunned() const { return bIsStunned; }

	UFUNCTION(BlueprintPure, Category = "AO|Animation|Werewolf")
	float GetSpeed() const { return Speed; }

	UFUNCTION(BlueprintPure, Category = "AO|Animation|Werewolf")
	bool IsMoving() const { return bIsMoving; }

	// 기절 몽타주 정보 Getter (Multicast용)
	UFUNCTION(BlueprintPure, Category = "AO|Animation|Werewolf")
	UAnimMontage* GetStunMontage() const { return StunMontage; }

	UFUNCTION(BlueprintPure, Category = "AO|Animation|Werewolf")
	float GetStunMontagePlayRate() const { return StunMontagePlayRate; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|Animation|Werewolf")
	TObjectPtr<AAO_Werewolf> WerewolfCharacter;

	// === Locomotion 변수 (블루프린트용) ===

	UPROPERTY(BlueprintReadOnly, Category = "AO|Animation|Werewolf|Locomotion")
	float Speed = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "AO|Animation|Werewolf|Locomotion")
	bool bIsMoving = false;

	// === 상태 플래그 ===

	// 기절 상태
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|Animation|Werewolf|State")
	bool bIsStunned = false;

	// === 몽타주 설정 ===

	// 기절 몽타주 (에디터에서 할당)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Animation|Werewolf|Montage")
	TObjectPtr<UAnimMontage> StunMontage;

	// 기절 몽타주 재생 속도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Animation|Werewolf|Montage")
	float StunMontagePlayRate = 1.0f;
};

