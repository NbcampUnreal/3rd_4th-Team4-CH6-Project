// KSJ : AO_Enemy_Troll.h

#pragma once

#include "CoreMinimal.h"
#include "AI/Character/AO_EnemyBase.h"
#include "AO_Enemy_Troll.generated.h"

/**
 * 트롤(Troll) AI 적
 * - 덩치가 크고 느림 (평상시 300, 추격 500)
 * - 근접 공격 (넉백 유발)
 * - GAS AttributeSet(Health, AttackDamage 등) 활용
 */
UCLASS()
class AO_API AAO_Enemy_Troll : public AAO_EnemyBase
{
	GENERATED_BODY()

public:
	AAO_Enemy_Troll();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void PerformAttack();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetMovementMode(bool bIsChasing);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float PatrolSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float ChaseSpeed = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float AttackRange = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float AttackDamageAmount = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float KnockbackStrength = 1500.0f;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	FGameplayTag AttackAbilityTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TSubclassOf<class UGameplayEffect> KnockbackEffectClass;

private:
	void ApplyAttackEffect(AActor* Target);
};