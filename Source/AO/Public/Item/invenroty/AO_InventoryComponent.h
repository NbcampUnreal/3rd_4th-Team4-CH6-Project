#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AO_InventoryComponent.generated.h"

USTRUCT(BlueprintType)
struct FInventorySlot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID = "empty";

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Quantity = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FuelAmount = 0.f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryUpdated, const TArray<FInventorySlot>&, Slots);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AO_API UAO_InventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAO_InventoryComponent();
	
	UPROPERTY(ReplicatedUsing=OnRep_Slots, EditAnywhere, BlueprintReadWrite, Category="Inventory")
	TArray<FInventorySlot> Slots;
	
	UPROPERTY(ReplicatedUsing=OnRep_SelectedIndex, BlueprintReadWrite, Category="Inventory")
	int32 SelectedSlotIndex = 1;
	
	UPROPERTY(BlueprintAssignable, Category="Inventory")
	FOnInventoryUpdated OnInventoryUpdated;
	
	UFUNCTION(Server, Reliable)
	void ServerSetSelectedSlot(int32 NewIndex);
	void ServerSetSelectedSlot_Implementation(int32 NewIndex);
		
	void PickupItem(const FInventorySlot& IncomingItem, AActor* Instigator);
	void UseItem();
	void DropItem();
	
	UFUNCTION(BlueprintPure, Category="Inventory")
	TArray<FInventorySlot> GetSlots() const { return Slots; }

	void ClearSlot();
	
protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION()
	void OnRep_Slots();

	UFUNCTION()
	void OnRep_SelectedIndex();
	
	bool IsValidSlotIndex(int32 Index) const { return Index >= 0 && Index < Slots.Num(); }
};
