#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "AO_InventorySubsystem.generated.h"

class UAO_InventoryComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnInventoryRegistered,
	UAO_InventoryComponent*, InvenComp
);

UCLASS()
class AO_API UAO_InventorySubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	void Initialize(FSubsystemCollectionBase& Collection);
	UFUNCTION()
	void HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);

	void RegisterInventory(UAO_InventoryComponent* InvenComp);

	UFUNCTION(BlueprintCallable)
	UAO_InventoryComponent* GetInvenComp() const;

	UPROPERTY(BlueprintAssignable)
	FOnInventoryRegistered OnInvenRegistered;

private:
	UPROPERTY()
	TObjectPtr<UAO_InventoryComponent> CachedInvenComp;
};
