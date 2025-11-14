// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AO_AddFuel_GameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class AO_API UAO_AddFuel_GameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UAO_AddFuel_GameplayAbility();

	virtual void ActivateAbility(
	   const FGameplayAbilitySpecHandle Handle,
	   const FGameplayAbilityActorInfo* ActorInfo,
	   const FGameplayAbilityActivationInfo ActivationInfo,
	   const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditDefaultsOnly, Category="GAS|Effects")
	TSubclassOf<UGameplayEffect> AddEnergyEffectClass;

	// 외부에서 세팅할 수 있는 임시 값 (인스턴스 단위)
	UPROPERTY(Transient)
	float PendingAmount = 0.f;
};

