//KSJ : AO_GA_Werewolf_Attack

#pragma once

#include "CoreMinimal.h"
#include "AI/GAS/Ability/AO_GA_AIAttackBase.h"
#include "AO_GA_Werewolf_Attack.generated.h"

class UGameplayEffect;

/**
 * Werewolf Attack Ability
 * - 근접 공격 (Heavy Hit)
 * - AO_GA_AIAttackBase를 상속하여 히트 처리 로직 사용
 */
UCLASS()
class AO_API UAO_GA_Werewolf_Attack : public UAO_GA_AIAttackBase
{
	GENERATED_BODY()

public:
	UAO_GA_Werewolf_Attack();
	
protected:
	// AO_GA_AIAttackBase의 OnTargetHit 오버라이드 (Heavy Hit React 적용)
	virtual void OnTargetHit(AActor* TargetActor, AActor* InstigatorActor) override;

	// AO_GA_AIAttackBase의 ApplyDamageAndKnockback 오버라이드 (DamageEffectClass 사용)
	virtual void ApplyDamageAndKnockback(AActor* TargetActor, AActor* InstigatorActor, const FEnemyAttackConfig& Config) override;

protected:
	// 데미지 Effect 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|AI|Werewolf")
	TSubclassOf<UGameplayEffect> DamageEffectClass;
};
