// AO_GA_AI_Stun.h

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AO_GA_AI_Stun.generated.h"

/**
 * AI 기절 처리 Ability
 * 
 * - Event.AI.Stunned 이벤트로 트리거
 * - 기절 모션 재생
 * - 기절 중 이동 비활성화
 * - 기절 종료 시 복구
 */
UCLASS()
class AO_API UAO_GA_AI_Stun : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UAO_GA_AI_Stun();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();

protected:
	// 기절 모션
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|AI|Stun")
	TObjectPtr<UAnimMontage> StunMontage;

	// 기절 중 적용할 Effect (Status.Debuff.Stunned 태그 부여)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|AI|Stun")
	TSubclassOf<UGameplayEffect> StunEffectClass;

	// 기절 지속 시간 (Effect에서 설정하지 않은 경우 사용)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|AI|Stun")
	float DefaultStunDuration = 3.f;

private:
	FActiveGameplayEffectHandle StunEffectHandle;
};
