#include "Item/invenroty/AO_InventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Actor.h"
#include "Item/AO_MasterItem.h"
#include "Kismet/GameplayStatics.h"

UAO_InventoryComponent::UAO_InventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	
	Slots.SetNum(5);
	for (FInventorySlot& Slot : Slots)
	{
		Slot = FInventorySlot();
	}

	SelectedSlotIndex = 1;
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
	//
}

void UAO_InventoryComponent::PickupItem(const FInventorySlot& IncomingItem, AActor* Instigator)
{
	if (!IsValidSlotIndex(SelectedSlotIndex)) return;
	if (GetOwnerRole() != ROLE_Authority) return;

	AAO_MasterItem* WorldItemActor = Cast<AAO_MasterItem>(Instigator);
	if (!WorldItemActor) return;

	FName PreviousItemID = Slots[SelectedSlotIndex].ItemID;

	Slots[SelectedSlotIndex] = IncomingItem;
	OnInventoryUpdated.Broadcast(Slots);

	if (PreviousItemID != "empty")
	{
		FVector SpawnLocation = GetOwner()->GetActorLocation() + GetOwner()->GetActorForwardVector() * 40.f;
		FRotator SpawnRotation = FRotator::ZeroRotator;
		FTransform SpawnTransform(SpawnRotation, SpawnLocation);
		
		AAO_MasterItem* DropItem = GetWorld()->SpawnActorDeferred<AAO_MasterItem>(
			AAO_MasterItem::StaticClass(),
			SpawnTransform,
			nullptr,
			nullptr,
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn
		);

		if (DropItem)
		{
			DropItem->ItemID = PreviousItemID;
			
			if (WorldItemActor->ItemDataTable)
			{
				DropItem->ItemDataTable = WorldItemActor->ItemDataTable;
				UE_LOG(LogTemp, Warning, TEXT("[SpawnActorDeferred] ItemDataTable Assigned successfully."));
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("[SpawnActorDeferred] WorldItemActor ItemDataTable is NULL, cannot assign."));
			}

			UGameplayStatics::FinishSpawningActor(DropItem, SpawnTransform);
		}
	}
	else
	{
		// 원래 있던 월드 아이템 제거
		WorldItemActor->Destroy();
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