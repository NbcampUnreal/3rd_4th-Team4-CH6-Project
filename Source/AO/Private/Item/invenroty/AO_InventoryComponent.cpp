#include "Item/invenroty/AO_InventoryComponent.h"

#include "EnhancedInputComponent.h"
#include "Character/AO_PlayerCharacter.h"
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

void UAO_InventoryComponent::SetupInputBinding(UInputComponent* PlayerInputComponent)
{
	if (!PlayerInputComponent)
	{
		return;
	}

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EIC)
	{
		return;
	}
	if (IA_Select_inventory_Slot)
	{
		EIC->BindAction(IA_Select_inventory_Slot, ETriggerEvent::Started, this, &UAO_InventoryComponent::SelectInventorySlot);
	}
	if (IA_UseItem)
	{
		EIC->BindAction(IA_UseItem, ETriggerEvent::Started, this, &UAO_InventoryComponent::UseInvenrotyItem);
	}
	if (IA_DropItem)
	{
		EIC->BindAction(IA_DropItem, ETriggerEvent::Started, this, &UAO_InventoryComponent::DropInvenrotyItem);	
	}
}

void UAO_InventoryComponent::ServerSetSelectedSlot_Implementation(int32 NewIndex)
{
	if (!IsValidSlotIndex(NewIndex))
	{
		//UE_LOG(LogTemp, Warning, TEXT("SERVER: Invalid slot index %d"), NewIndex);
		return;
	}

	SelectedSlotIndex = NewIndex;
	
	if (OnInventoryUpdated.IsBound())
	{
		OnInventoryUpdated.Broadcast(Slots);
	}

	//UE_LOG(LogTemp, Verbose, TEXT("SERVER: SelectedSlotIndex set to %d"), SelectedSlotIndex);
}

void UAO_InventoryComponent::PickupItem(const FInventorySlot& IncomingItem, AActor* Instigator)
{
	if (!IsValidSlotIndex(SelectedSlotIndex)) return;
	if (GetOwnerRole() != ROLE_Authority) return;

	AAO_MasterItem* WorldItemActor = Cast<AAO_MasterItem>(Instigator);
	if (!WorldItemActor) return;
	
	FInventorySlot OldSlot = Slots[SelectedSlotIndex];
	
	Slots[SelectedSlotIndex] = IncomingItem;
	Slots[SelectedSlotIndex].ItemType = IncomingItem.ItemType;
	Slots[SelectedSlotIndex].FuelAmount = IncomingItem.FuelAmount;
	OnInventoryUpdated.Broadcast(Slots);
	
	
	if (OldSlot.ItemID != "empty")
	{
		FVector SpawnLocation = GetOwner()->GetActorLocation() + GetOwner()->GetActorForwardVector() * 40.f;
		FRotator SpawnRotation = FRotator::ZeroRotator;
		FTransform SpawnTransform(SpawnRotation, SpawnLocation);

		AAO_MasterItem* DropItem = GetWorld()->SpawnActorDeferred<AAO_MasterItem>(
			DroppableItemClass ? DroppableItemClass.Get() : AAO_MasterItem::StaticClass(),
			SpawnTransform,
			nullptr,
			nullptr,
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn
		);

		if (DropItem)
		{
			DropItem->ItemID = OldSlot.ItemID;
			UGameplayStatics::FinishSpawningActor(DropItem, SpawnTransform);
		}
	}
	else
	{
		WorldItemActor->Destroy();
	}
}



void UAO_InventoryComponent::UseInvenrotyItem()
{
	//
}

void UAO_InventoryComponent::DropInvenrotyItem()
{
	if (!IsValidSlotIndex(SelectedSlotIndex)) return;
	if (GetOwnerRole() != ROLE_Authority) return;
	
	const FInventorySlot& CurrentSlot = Slots[SelectedSlotIndex];

	if (CurrentSlot.ItemID != "empty")
	{
		FVector SpawnLocation = GetOwner()->GetActorLocation() + GetOwner()->GetActorForwardVector() * 40.f;
		FRotator SpawnRotation = FRotator::ZeroRotator;
		FTransform SpawnTransform(SpawnRotation, SpawnLocation);
		
		AAO_MasterItem* DropItem = GetWorld()->SpawnActorDeferred<AAO_MasterItem>(
		DroppableItemClass ? DroppableItemClass.Get() : AAO_MasterItem::StaticClass(), 
		SpawnTransform,
		 nullptr,
		 nullptr,
		 ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn
	   );

		if (DropItem)
		{
			DropItem->ItemID = CurrentSlot.ItemID;
			// DropItem->FuelAmount = CurrentSlot.FuelAmount; 

			UGameplayStatics::FinishSpawningActor(DropItem, SpawnTransform);
		}
	}

	ClearSlot();
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

//ms_inventory key binding
void UAO_InventoryComponent::SelectInventorySlot(const FInputActionValue& Value)
{
	//UE_LOG(LogTemp, Warning, TEXT("INPUT BINDING SUCCESS! Raw Slot Value (Float): %f"), Value.Get<float>());
	
	float SlotIndexAsFloat = Value.Get<float>();
	int32 SlotIndex = FMath::RoundToInt(SlotIndexAsFloat); 
	
	ServerSetSelectedSlot(SlotIndex);
	
}
