#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "Components/ActorComponent.h"
#include "Item/AO_struct_FItemBase.h"
#include "AO_InventoryComponent.generated.h"

class AAO_MasterItem;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EItemType ItemType = EItemType::None;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryUpdated, const TArray<FInventorySlot>&, Slots);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSelectSlotUpdated, const int32, SelectedSlotIndex);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AO_API UAO_InventoryComponent : public UActorComponent
{
	GENERATED_BODY()


	
public:
	UAO_InventoryComponent();
	
	void SetupInputBinding(UInputComponent* PlayerInputComponent);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Select_inventory_Slot;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_UseItem;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_DropItem;
	
	UPROPERTY(ReplicatedUsing=OnRep_Slots, EditAnywhere, BlueprintReadWrite, Category="Inventory")
	TArray<FInventorySlot> Slots;

	
	UPROPERTY(ReplicatedUsing=OnRep_SelectedIndex, BlueprintReadWrite, Category="Inventory")
	int32 SelectedSlotIndex = 1;

	UPROPERTY(EditDefaultsOnly, Category="Inventory") 
	TSubclassOf<AAO_MasterItem> DroppableItemClass;
	
	UPROPERTY(BlueprintAssignable, Category="Inventory")
	FOnInventoryUpdated OnInventoryUpdated;

	UPROPERTY(BlueprintAssignable, Category="Inventory")
	FOnSelectSlotUpdated OnSelectSlotUpdated;
	
	UFUNCTION(Server, Reliable)
	void ServerSetSelectedSlot(int32 NewIndex);
	void ServerSetSelectedSlot_Implementation(int32 NewIndex);

	void OnRightClick();
	void OnLeftClick();
	
	void SelectInventorySlot(const FInputActionValue& Value);
	void PickupItem(const FInventorySlot& IncomingItem, AActor* Instigator);
	UFUNCTION(Server, Reliable)
	void UseInventoryItem_Server();
	UFUNCTION(Server, Reliable)
	void DropInventoryItem_Server();
	
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
