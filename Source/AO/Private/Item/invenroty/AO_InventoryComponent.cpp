#include "Item/invenroty/AO_InventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Actor.h"

UAO_InventoryComponent::UAO_InventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	
	Slots.SetNum(4);
	for (FInventorySlot& Slot : Slots)
	{
		Slot = FInventorySlot();
	}

	SelectedSlotIndex = 0;
}

void UAO_InventoryComponent::ServerSetSelectedSlot_Implementation(int32 NewIndex)
{
	if (!IsValidSlotIndex(NewIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("SERVER: Invalid slot index %d"), NewIndex);
		return;
	}

	SelectedSlotIndex = NewIndex;
	
	if (OnInventoryUpdated.IsBound())
	{
		OnInventoryUpdated.Broadcast(Slots);
	}

	UE_LOG(LogTemp, Verbose, TEXT("SERVER: SelectedSlotIndex set to %d"), SelectedSlotIndex);
}

void UAO_InventoryComponent::UseSelectedItem()
{
	/*if (GetOwnerRole() == ROLE_Authority)
	{
		if (!IsValidSlotIndex(SelectedSlotIndex)) return;

		//수량감소
		FInventorySlot& Slot = Slots[SelectedSlotIndex];
		if (Slot.Quantity > 0)
		{
			Slot.Quantity = FMath::Max(0, Slot.Quantity - 1);
			
			if (OnInventoryUpdated.IsBound())
			{
				OnInventoryUpdated.Broadcast(Slots);
			}
		}
	}
	*/
}

void UAO_InventoryComponent::PickupItem(const FInventorySlot& IncomingItem)
{
	if (!IsValidSlotIndex(SelectedSlotIndex)) return;

	if (GetOwnerRole() == ROLE_Authority)
	{
		Slots[SelectedSlotIndex] = IncomingItem;
		UE_LOG(LogTemp, Log, TEXT("SERVER: Picked up item into slot %d, FuelAmount=%f"),
			SelectedSlotIndex, IncomingItem.FuelAmount)
		
		if (OnInventoryUpdated.IsBound())
		{
			OnInventoryUpdated.Broadcast(Slots);
		}
	}
	else
	{
		// 클라이언트가 Pickup을 시도하면 서버 RPC로 요청해야 함. (예: ServerPickupItem)
	}
}

void UAO_InventoryComponent::OnRep_Slots()
{
	//UE_LOG(LogTemp, Verbose, TEXT("CLIENT: OnRep_Slots called (Owner: %s)"), *GetOwner()->GetName());
	
	if (OnInventoryUpdated.IsBound())
	{
		OnInventoryUpdated.Broadcast(Slots);
	}
}

void UAO_InventoryComponent::OnRep_SelectedIndex()
{
	/*UE_LOG(LogTemp, Verbose, TEXT("CLIENT: SelectedSlotIndex updated to: %d (Owner: %s)"),
		   SelectedSlotIndex,
		   *GetOwner()->GetName());*/
}

void UAO_InventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UAO_InventoryComponent, Slots);
	DOREPLIFETIME(UAO_InventoryComponent, SelectedSlotIndex);
}

void UAO_InventoryComponent::ClearSlot()
{
	Slots[SelectedSlotIndex] = FInventorySlot();
	
	if (OnInventoryUpdated.IsBound())
	{
		OnInventoryUpdated.Broadcast(Slots);
	}
}