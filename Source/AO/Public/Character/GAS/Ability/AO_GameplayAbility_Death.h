// AO_GameplayAbility_Death.h

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AO_GameplayAbility_Death.generated.h"

UCLASS()
class AO_API UAO_GameplayAbility_Death : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UAO_GameplayAbility_Death();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Death")
	TObjectPtr<UAnimMontage> DeathMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HitReact")
	TSubclassOf<UGameplayEffect> BlockAbilitiesEffectClass;

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();
};
