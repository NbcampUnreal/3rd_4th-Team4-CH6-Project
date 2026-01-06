#include "Item/AO_MasterItem.h"
#include "Components/StaticMeshComponent.h"
#include "AbilitySystemComponent.h"
#include "Item/AO_struct_FItemBase.h"
#include "Public/Item/inventory/AO_InventoryComponent.h"
#include "Net/UnrealNetwork.h"

AAO_MasterItem::AAO_MasterItem()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	if (MeshComponent)
	{
		MeshComponent->SetIsReplicated(true);
		MeshComponent->SetSimulatePhysics(true);        
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
		MeshComponent->SetEnableGravity(true);
	}

	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(MeshComponent);
	InteractionSphere->SetSphereRadius(50.f);
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	MeshComponent->SetSimulatePhysics(true);        
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
	MeshComponent->SetEnableGravity(true);
}

void AAO_MasterItem::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && !ItemID.IsNone())
	{
		ApplyItemData();
	}
}

void AAO_MasterItem::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

#if WITH_EDITOR
	if (!HasAuthority())
	{
		return;
	}

	if (!ItemID.IsNone())
	{
		ApplyItemData();
	}
#endif
}

FAO_InteractionInfo AAO_MasterItem::GetInteractionInfo(const FAO_InteractionQuery& InteractionQuery) const
{
	FAO_InteractionInfo Info = Super::GetInteractionInfo(InteractionQuery);
    
	// ItemData의 ItemName으로 Title 변경
	if (!CachedItemName.IsEmpty())
	{
		Info.Title = FText::FromString(CachedItemName);
	}
    
	return Info;
}

void AAO_MasterItem::OnRep_ItemID()
{
	if (!HasActorBegunPlay())
	{
		return;
	}
	if (!ItemID.IsNone())
	{
		ApplyItemData();
	}
}

void AAO_MasterItem::ApplyItemData()
{
	if (!ItemDataTable)
	{
		return;
	}
	if (ItemID.IsNone())
	{
		return;
	}

	static const FString Context(TEXT("Item Lookup"));

	if (const FAO_struct_FItemBase* Row = ItemDataTable->FindRow<FAO_struct_FItemBase>(ItemID, Context))
	{
		ItemTags = Row->ItemTags;
		FuelAmount = Row->FuelAmount;
		HighlightStencilValue = Row->GetHighlightStencilValue();
		CachedItemName = Row->ItemName;
		
		if (!Row->WorldMesh.IsNull())
		{
			if (UStaticMesh* Mesh = Row->WorldMesh.LoadSynchronous())
			{
				if (MeshComponent && Mesh)
				{
					if (MeshComponent->GetStaticMesh() != Mesh)
					{
						MeshComponent->SetStaticMesh(Mesh);
					}
				}
			}
		}
	}
}



void AAO_MasterItem::OnInteractionSuccess_BP_Implementation(AActor* Interactor)
{
	Super::OnInteractionSuccess_BP_Implementation(Interactor);

	if (!HasAuthority())
	{
		Server_HandleInteraction(Interactor);
		return;
	}
	Server_HandleInteraction(Interactor);
}

void AAO_MasterItem::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	TagContainer = ItemTags;
}

void AAO_MasterItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAO_MasterItem, ItemTags);
	DOREPLIFETIME(AAO_MasterItem, FuelAmount);
	DOREPLIFETIME(AAO_MasterItem, ItemID);	
	DOREPLIFETIME(AAO_MasterItem, CachedItemName);
}

void AAO_MasterItem::Server_HandleInteraction_Implementation(AActor* Interactor)
{
	if (!Interactor) return;

	UAO_InventoryComponent* Inventory = Interactor->FindComponentByClass<UAO_InventoryComponent>();
	if (!Inventory) return;

	FInventorySlot ItemToAdd;
	ItemToAdd.ItemID = this->ItemID;
	ItemToAdd.Quantity = 1;
	
	float ServerFuelAmount = FuelAmount;
	ItemToAdd.FuelAmount = ServerFuelAmount;

	if (ItemDataTable)
	{
		static const FString Context(TEXT("Item Lookup"));
		if (const FAO_struct_FItemBase* Row = ItemDataTable->FindRow<FAO_struct_FItemBase>(ItemID, Context))
		{
			ItemToAdd.ItemType = Row->ItemType;
		}
	}
	
	Inventory->PickupItem(ItemToAdd, this);

	Destroy();
}