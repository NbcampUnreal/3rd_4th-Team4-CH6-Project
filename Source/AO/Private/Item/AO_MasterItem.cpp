#include "Item/AO_MasterItem.h"
#include "Components/StaticMeshComponent.h"
#include "AbilitySystemComponent.h"
#include "Item/AO_struct_FItemBase.h"
#include "Item/invenroty/AO_InventoryComponent.h"
#include "Net/UnrealNetwork.h"

AAO_MasterItem::AAO_MasterItem()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	if (MeshComponent)
	{
		MeshComponent->SetIsReplicated(true);
	}

	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(MeshComponent);
	InteractionSphere->SetSphereRadius(50.f);
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	PickupComponent = CreateDefaultSubobject<UAO_PickupComponent>(TEXT("PickupComp"));

	MeshComponent->SetSimulatePhysics(true);        
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
	MeshComponent->SetEnableGravity(true);
}

void AAO_MasterItem::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		ApplyItemData();
	}
}

void AAO_MasterItem::OnRep_ItemID()
{
	ApplyItemData();
}

void AAO_MasterItem::ApplyItemData()
{
	if (!ItemDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("ApplyItemData failed: ItemDataTable is NULL. ItemID: %s"), *ItemID.ToString());
		return;
	}
	if (ItemID.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("ApplyItemData failed: ItemID is None."));
		return;
	}

	static const FString Context(TEXT("Item Lookup"));

	if (const FAO_struct_FItemBase* Row = ItemDataTable->FindRow<FAO_struct_FItemBase>(ItemID, Context))
	{
		//UE_LOG(LogTemp, Warning, TEXT("ApplyItemData SUCCESS for ItemID: %s"), *ItemID.ToString());
		ItemTags = Row->ItemTags;
		FuelAmount = Row->FuelAmount;
		
		if (!Row->WorldMesh.IsNull())
		{
			if (UStaticMesh* Mesh = Row->WorldMesh.LoadSynchronous())
			{
				if (MeshComponent)
				{
					MeshComponent->SetStaticMesh(Mesh);
				}
			}
		}
	}
	else
	{
		//UE_LOG(LogTemp, Error, TEXT("ApplyItemData FAILED to find row for ItemID: %s"), *ItemID.ToString());
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

	//UE_LOG(LogTemp, Warning, TEXT("DEBUG: Adding item to inventory. FuelAmount = %f"), ServerFuelAmount);
	Inventory->PickupItem(ItemToAdd, this);

	Destroy();
}

void AAO_MasterItem::ItemSawp(FName NewItemID)
{
	//
}