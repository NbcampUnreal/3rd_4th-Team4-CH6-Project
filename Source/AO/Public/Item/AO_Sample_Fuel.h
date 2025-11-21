// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UObject/SoftObjectPtr.h"
#include "AO_Sample_Fuel.generated.h"

class AAO_Train;
class UAO_AddFuel_GameplayAbility;

UCLASS()
class AO_API AAO_Sample_Fuel : public AActor
{
	GENERATED_BODY()

public:
	AAO_Sample_Fuel();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS")
	float AddFuelAmount = 50.f;

	// Soft pointer: 에디터에서 BP Ability 를 할당하려면 이걸 쓰는게 안전
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS")
	TSoftClassPtr<UAO_AddFuel_GameplayAbility> AddEnergyAbilityClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UStaticMeshComponent> FuelMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS")
	AAO_Train* TargetTrain = nullptr;

private:
	UPROPERTY()
	bool bConsumed = false;

	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* Overlapped, AActor* OtherActor,
				   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
				   bool bFromSweep, const FHitResult& SweepResult);
};

