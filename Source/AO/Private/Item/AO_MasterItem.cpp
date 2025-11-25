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
	MeshComponent->SetIsReplicated(true);

	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(MeshComponent);
	InteractionSphere->SetSphereRadius(50.f);
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void AAO_MasterItem::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && !ItemID.IsNone() && ItemDataTable)
	{
		static const FString ContextString(TEXT("Reading ItemDataTable"));
		if (FAO_struct_FItemBase* Row = ItemDataTable->FindRow<FAO_struct_FItemBase>(ItemID, ContextString))
		{
			ItemTags = Row->ItemTags;
			FuelAmount = Row->FuelAmount;
			if (!Row->WorldMesh.IsNull())
			{
				if (UStaticMesh* Mesh = Row->WorldMesh.LoadSynchronous())
					MeshComponent->SetStaticMesh(Mesh);
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