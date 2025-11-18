// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/Base/AO_BaseInteractable.h"
#include "Train/AO_Train.h"
#include "Train/GAS/AO_AddFuel_GameplayAbility.h"
#include "AO_Fuel_interactable.generated.h"

UCLASS()
class AO_API AAO_Fuel_interactable : public AAO_BaseInteractable
{
	GENERATED_BODY()

public:
	AAO_Fuel_interactable();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS")
	float AddFuelAmount = 50.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS")
	TSoftClassPtr<UAO_AddFuel_GameplayAbility> AddEnergyAbilityClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS")
	AAO_Train* TargetTrain = nullptr;

	//interect
	virtual void OnInteractionSuccess_BP_Implementation(AActor* Interactor) override;

	//monster can pickup
	UFUNCTION()
	void OnRep_Holder();

	UFUNCTION()
	void OnRep_IsPickedUp();

	void ApplyPickedUpVisuals();
	void ApplyDroppedVisuals();

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup")
	class UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup")
	class USphereComponent* SphereComponent;

	UPROPERTY(ReplicatedUsing = OnRep_IsPickedUp, VisibleAnywhere, BlueprintReadOnly, Category = "Pickup")
	bool bIsPickedUp;

	UPROPERTY(EditAnywhere, Category = "Pickup")
	bool bDropWithPhysics = false;

	UPROPERTY(ReplicatedUsing = OnRep_Holder)
	AActor* HolderActor = nullptr;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Tags")
	FGameplayTagContainer ItemTags;

	UFUNCTION(BlueprintCallable, Category = "Tags")
	void AddGameplayTag(FGameplayTag Tag);

	UFUNCTION(BlueprintCallable, Category = "Tags")
	void RemoveGameplayTag(FGameplayTag Tag);

	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;

	UFUNCTION(BlueprintCallable, Category = "Pickup")
	void Pickup(USceneComponent* AttachTo, FName SocketName);

	UFUNCTION(BlueprintCallable, Category = "Pickup")
	void Drop();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
private:
	UPROPERTY()
	bool bConsumed = false;
};
