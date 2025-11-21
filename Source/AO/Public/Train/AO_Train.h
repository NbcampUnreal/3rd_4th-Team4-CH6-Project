// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Actor.h"
#include "Abilities/GameplayAbility.h"
#include "AO_Train.generated.h"

class UAbilitySystemComponent;
class UAO_Fuel_AttributeSet;

UCLASS()
class AO_API AAO_Train : public AActor,  public IAbilitySystemInterface

{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AAO_Train();
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
	TObjectPtr<UStaticMeshComponent> StaticMesh;
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayAbility> AddEnergyAbilityClass;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> ASC;

	UPROPERTY()
	UAO_Fuel_AttributeSet* FuelAttributeSet;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fuel")
	float TotalFuelGained = 0.f;

	virtual void BeginPlay() override;
	void OnFuelChanged(const FOnAttributeChangeData& Data);
};
