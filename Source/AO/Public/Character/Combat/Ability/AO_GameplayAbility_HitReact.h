// AO_GameplayAbility_HitReact.h

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AO_GameplayAbility_HitReact.generated.h"

/**
 * 피격 리액션 전용 Ability
 * - Event.Combat.HitReact 이벤트를 트리거로 발동
 */
UCLASS()
class AO_API UAO_GameplayAbility_HitReact : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UAO_GameplayAbility_HitReact();

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

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HitReact")
	TObjectPtr<UAnimMontage> DefaultHitReactMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HitReact")
	TMap<FGameplayTag, TObjectPtr<UAnimMontage>> HitReactMontageMap;

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();

	FString GetHitDirectionSuffix(const FGameplayEventData* TriggerEventData, const FGameplayAbilityActorInfo* ActorInfo) const;
};
