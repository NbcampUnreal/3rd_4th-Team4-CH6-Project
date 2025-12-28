#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Item/invenroty/AO_InventoryComponent.h" 
#include "AO_InventoryListener.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UAO_InventoryListener : public UInterface
{
	GENERATED_BODY()
};

class AO_API IAO_InventoryListener
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, Category = "Inventory")
	void OnSlotChanged(const TArray<FInventorySlot>& NewSlots);

	UFUNCTION(BlueprintNativeEvent, Category = "Inventory")
	void OnSelectChanged(int32 NewIndex);
};