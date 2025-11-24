// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AO_RemoveFuel_GameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class AO_API UAO_RemoveFuel_GameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

	public:
		UAO_RemoveFuel_GameplayAbility();
	
	UPROPERTY(EditDefaultsOnly, Category="GAS|Effects")
	TSubclassOf<UGameplayEffect> DeleteEnergyEffectClass;
	
	UPROPERTY(EditDefaultsOnly, Category="GAS|Effects")
	float PendingAmount = -10.f;

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
		bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	FActiveGameplayEffectHandle ActiveGEHandle;
};
