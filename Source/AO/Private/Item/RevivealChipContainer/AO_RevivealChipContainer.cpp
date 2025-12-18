#include "Item/RevivealChipContainer/AO_RevivealChipContainer.h"

#include "Item/invenroty/AO_InventoryComponent.h"

AAO_RevivealChipContainer::AAO_RevivealChipContainer()
{
}

void AAO_RevivealChipContainer::BeginPlay()
{
	Super::BeginPlay();
	
}

void AAO_RevivealChipContainer::OnInteractionSuccess(AActor* Interactor)
{
	if (!HasAuthority()) 
	{
		return;
	}

	UAO_InventoryComponent* Inventory = Interactor->FindComponentByClass<UAO_InventoryComponent>();
	if (!Inventory) return;

	if (!Inventory->Slots.IsValidIndex(Inventory->SelectedSlotIndex))
	{
		return;
	}

	FInventorySlot& Slot = Inventory->Slots[Inventory->SelectedSlotIndex];

	EItemType ItemType = Slot.ItemType;
	//UE_LOG(LogTemp, Warning, TEXT("Slot ItemType: %d, ItemID: %s"), (int32)Slot.ItemType, *Slot.ItemID.ToString());
	
	if (ItemType != EItemType::RevivalChip)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Revival Chip used!"));
	Inventory->ClearSlot();	
}