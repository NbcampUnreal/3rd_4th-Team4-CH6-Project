// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/AO_Fuel_AttributeSet.h"
#include "Interaction/Base/AO_BaseInteractable.h"
#include "AO_newTrain.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFuelChange, float, NewFuel);

UCLASS()
class AO_API AAO_newTrain : public AAO_BaseInteractable
{
	GENERATED_BODY()

public:
	AAO_newTrain();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|Abilities")
	void FuelLeakSkillOn();
	UFUNCTION(BlueprintCallable, Category = "GAS|Abilities")
	void FuelLeakSkillOut();

	UPROPERTY(BlueprintAssignable, Category="Fuel")
	FOnFuelChange OnFuelChangedDelegate;
protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> ASC;
	
	UPROPERTY()
	UAO_Fuel_AttributeSet* FuelAttributeSet;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayAbility> AddEnergyAbilityClass;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayAbility> LeakEnergyAbilityClass;

	virtual void OnInteractionSuccess(AActor* Interactor) override;
	void OnFuelChange(const FOnAttributeChangeData& Data);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fuel")
	float TotalFuelGained = 0.f;
};
