#include "Item/RevivealChipContainer/AO_RevivealChipContainer.h"

#include "Game/GameInstance/AO_GameInstance.h"
#include "Game/GameState/AO_GameState.h"
#include "Public/Item/inventory/AO_InventoryComponent.h"

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

	UWorld* World = GetWorld();
	if (World != nullptr)
	{
		AAO_GameState* AO_GS = World->GetGameState<AAO_GameState>();
		if (AO_GS != nullptr)
		{
			AO_GS->AddSharedReviveCount(1);
		}
	}
	
	Inventory->ClearSlot();	
}