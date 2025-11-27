#include "Item/PassiveContainer/AO_PassiveContainer.h"
#include "GameplayTagContainer.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Interaction/Component/AO_InteractableComponent.h"
#include "Item/invenroty/AO_InventoryComponent.h"

AAO_PassiveContainer::AAO_PassiveContainer()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	SetRootComponent(StaticMesh);
	StaticMesh->SetIsReplicated(true);
	
	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));
	ASC->SetIsReplicated(true);
	ASC->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	
	InteractableComp = CreateDefaultSubobject<UAO_InteractableComponent>(TEXT("InteractableComponent"));
	if (InteractableComp)
	{
		InteractableComp->OnInteractionSuccess.AddDynamic(this, &AAO_PassiveContainer::HandleInteractionSuccess);
	}
	
}

void AAO_PassiveContainer::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		if (!ASC) return;

		ASC->InitAbilityActorInfo(this, this);
	}
}

void AAO_PassiveContainer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
}

UAbilitySystemComponent* AAO_PassiveContainer::GetAbilitySystemComponent() const
{
	return ASC;
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
	UE_LOG(LogTemp, Warning, TEXT("Slot ItemType: %d, ItemID: %s"), (int32)Slot.ItemType, *Slot.ItemID.ToString());
	
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
	
	const FGameplayTag ActivationEventTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Interaction.AddPassive")); 
    
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
