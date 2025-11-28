// KSJ : AO_Enemy_Werewolf.h

#pragma once

#include "CoreMinimal.h"
#include "AI/Character/AO_EnemyBase.h"
#include "AO_Enemy_Werewolf.generated.h"

/**
 * 웨어울프 캐릭터 본체
 * 
 * - 빠른 이동 속도
 * - 근접 공격 (몽타주 재생)
 */
UCLASS()
class AO_API AAO_Enemy_Werewolf : public AAO_EnemyBase
{
	GENERATED_BODY()

public:
	AAO_Enemy_Werewolf();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void PerformAttack();

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TObjectPtr<UAnimMontage> AttackMontage;
	
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TObjectPtr<UAnimMontage> HowlMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float AttackRange = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float AttackDamageAmount = 15.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TSubclassOf<class UGameplayEffect> AttackEffectClass;

private:
	void ApplyAttackEffect(AActor* Target);
};
