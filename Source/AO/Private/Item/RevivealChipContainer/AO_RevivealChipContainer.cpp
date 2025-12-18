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
	//UE_LOG(LogTemp, Warning, TEXT("Interaction Success!"));
	if (!HasAuthority()) 
	{
		return;
	}

	UAO_InventoryComponent* Inventory = Interactor->FindComponentByClass<UAO_InventoryComponent>();
	//UE_LOG(LogTemp, Warning, TEXT("Inventory Found!"));
	if (!Inventory) return;

	if (!Inventory->Slots.IsValidIndex(Inventory->SelectedSlotIndex))
	{
		//UE_LOG(LogTemp, Warning, TEXT("Invalid Slot Index!"));
		return;
	}

	FInventorySlot& Slot = Inventory->Slots[Inventory->SelectedSlotIndex];

	EItemType ItemType = Slot.ItemType;
	//UE_LOG(LogTemp, Warning, TEXT("Slot ItemType: %d, ItemID: %s"), (int32)Slot.ItemType, *Slot.ItemID.ToString());
	
	if (ItemType != EItemType::RevivalChip)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Invalid Item Type!"));
		return;
	}

	if (auto* GI = Cast<UAO_GameInstance>(GetWorld()->GetGameInstance())) // YourGameInstance를 실제 게임 인스턴스 클래스명으로 변경
	{
		GI->AddSharedReviveCount(1);
	}
	
	//UE_LOG(LogTemp, Warning, TEXT("Revival Chip used!"));
	Inventory->ClearSlot();	
}