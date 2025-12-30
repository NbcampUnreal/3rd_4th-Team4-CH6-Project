#include "Item/RevivealChipContainer/AO_RevivealChipContainer.h"

#include "Game/GameInstance/AO_GameInstance.h"
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
	
	if (ItemType != EItemType::RevivalChip)
	{
		return;
	}

	if (auto* GI = Cast<UAO_GameInstance>(GetWorld()->GetGameInstance()))
	{
		GI->AddSharedReviveCount(1);
	}
	
	Inventory->ClearSlot();	
}