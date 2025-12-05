// AO_GameplayAbility_MeleeAttack_Base.h

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AO_GameplayAbility_MeleeAttack_Base.generated.h"

class UAbilityTask_PlayMontageAndWait;

/**
 * 기본 근접 공격 어빌리티.
 * - 몽타주 재생만 담당 (실제 데미지 판정은 AO_GameplayAbility_MeleeHitConfirm 에서 수행)
 */
UCLASS()
class AO_API UAO_GameplayAbility_MeleeAttack_Base : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UAO_GameplayAbility_MeleeAttack_Base();

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

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	TObjectPtr<UAnimMontage> AttackMontage;

protected:
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();
};
