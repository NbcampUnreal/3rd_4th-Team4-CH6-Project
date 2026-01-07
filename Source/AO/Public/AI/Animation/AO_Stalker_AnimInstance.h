//KSJ : AO_Stalker_AnimInstance

#pragma once

#include "CoreMinimal.h"
#include "AI/Animation/AO_AIAnimInstance.h"
#include "AO_Stalker_AnimInstance.generated.h"

class AAO_Stalker;
class UAnimMontage;

/**
 * Stalker AI 애니메이션 인스턴스
 */
UCLASS()
class AO_API UAO_Stalker_AnimInstance : public UAO_AIAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	// 공격 거리에 따른 몽타주 선택 (BP에서 호출)
	UFUNCTION(BlueprintCallable, Category = "Combat")
	UAnimMontage* GetAttackMontage() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
	TObjectPtr<AAO_Stalker> StalkerCharacter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bInCeiling;

	// 로컬 좌표계 속도 (X: 전진/후진, Y: 좌우)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	FVector LocalVelocity;

	// 전진/후진 속도 (양수: 전진, 음수: 후진) - BlendSpace 입력용
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float ForwardSpeed;

	// 좌우 속도 (양수: 오른쪽, 음수: 왼쪽) - BlendSpace 입력용
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float RightSpeed;

	// 공격 몽타주 목록 (BP에서 할당)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TArray<TObjectPtr<UAnimMontage>> AttackMontages;
};

