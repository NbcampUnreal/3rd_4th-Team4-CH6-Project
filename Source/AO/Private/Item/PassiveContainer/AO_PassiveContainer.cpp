#include "Item/PassiveContainer/AO_PassiveContainer.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Item/invenroty/AO_InventoryComponent.h"

AAO_PassiveContainer::AAO_PassiveContainer()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AAO_PassiveContainer::BeginPlay()
{
	Super::BeginPlay();
}

void AAO_PassiveContainer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
}

void AAO_PassiveContainer::HandleInteractionSuccess(AActor* Interactor)
{
	if (!HasAuthority() || !ASC) 
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

	if (ItemType != EItemType::Passive)
	{
		return;
	}

	static const FString Context(TEXT("Inventory Item Lookup"));
	float AddPassive = 0.0f;
	
	const FAO_struct_FItemBase* ItemData = ItemDataTable->FindRow<FAO_struct_FItemBase>(
		Slot.ItemID, 
		Context      
	);

	if (ItemData)
	{
		AddPassive = ItemData->PassiveAmount;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to find data for ItemID: %s"), *Slot.ItemID.ToString());
	}
	
	const FGameplayTag ActivationEventTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Interaction.AddPassive.HP")); 
    
	FGameplayEventData EventData;
	EventData.EventTag = ActivationEventTag;
	EventData.Target = this;
	EventData.Instigator = Interactor;
	EventData.EventMagnitude = AddPassive;
	
	ASC->HandleGameplayEvent(
	   ActivationEventTag, 
	   &EventData
	);

	Inventory->ClearSlot();	
}
