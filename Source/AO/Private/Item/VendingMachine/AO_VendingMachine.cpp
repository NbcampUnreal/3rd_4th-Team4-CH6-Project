#include "Item/VendingMachine/AO_VendingMachine.h"

#include "EngineUtils.h"
#include "Engine/DataTable.h"
#include "Interaction/Component/AO_InteractableComponent.h"
#include "Item/AO_MasterItem.h"
#include "Item/AO_struct_FItemBase.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(LogShop);

AAO_VendingMachine::AAO_VendingMachine()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	SetRootComponent(StaticMesh);
	StaticMesh->SetIsReplicated(true);
	ItemMesh->SetupAttachment(StaticMesh);
	
	InteractableComp = CreateDefaultSubobject<UAO_InteractableComponent>(TEXT("InteractableComponent"));
	if (InteractableComp)
	{
		InteractableComp->OnInteractionSuccess.AddDynamic(this, &AAO_VendingMachine::HandleInteractionSuccess);
	}
}

void AAO_VendingMachine::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		// ShopManager가 미지정이면 자동 검색
		if (!ShopManager)
		{
			for (TActorIterator<AAO_ShopManager> It(GetWorld()); It; ++It)
			{
				ShopManager = *It;
				UE_LOG(LogTemp, Warning, TEXT("[VM] Found ShopManager on server."));
				break;
			}
		}

		if (!ShopManager)
		{
			UE_LOG(LogTemp, Error, TEXT("[VM] FAILED to find ShopManager"));
		}

		ApplyItemData();
	}
}
void AAO_VendingMachine::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	ApplyItemData();
}

void AAO_VendingMachine::OnRep_ItemID()
{
	if (StaticMesh)
	{
		ApplyItemData();
	}
}


void AAO_VendingMachine::ApplyItemData()
{
	if (!ItemDataTable)
	{
		return;
	}
	if (MechineItemID.IsNone())
	{
		return;
	}

	static const FString Context(TEXT("Item Lookup"));

	if (const FAO_struct_FItemBase* Row = ItemDataTable->FindRow<FAO_struct_FItemBase>(MechineItemID, Context))
	{		
		if (!Row->WorldMesh.IsNull())
		{
			if (UStaticMesh* Mesh = Row->WorldMesh.LoadSynchronous())
			{
				if (ItemMesh && Mesh)
				{
					if (ItemMesh->GetStaticMesh() != Mesh)
					{
						ItemMesh->SetStaticMesh(Mesh);
					}
				}
			}
			if (int32 TableData = Row->ItemPrice)
			{
				ItemPrice = TableData;
			}
		}
	}
	else
	{
	}
}

void AAO_VendingMachine::HandleInteractionSuccess(AActor* Interactor)
{
	UE_LOG(LogShop, Warning, TEXT("[VM] HandleInteractionSuccess Called. Authority=%s"),
		HasAuthority() ? TEXT("TRUE") : TEXT("FALSE"));

	if (!HasAuthority())
	{
		UE_LOG(LogShop, Warning, TEXT("[VM] NO AUTHORITY → EXIT"));
		return;
	}

	if (!ShopManager)
	{
		UE_LOG(LogShop, Error, TEXT("[VM] ShopManager is NULL!"));
		return;
	}

	UE_LOG(LogShop, Warning, TEXT("[VM] Request Buy Item. Price=%d"), ItemPrice);

	ShopManager->Server_BuyItem(ItemPrice, this);
}


void AAO_VendingMachine::SpawnVendingItem()
{
	UE_LOG(LogShop, Warning, TEXT("[VM] SpawnVendingItem Executed"));

	FVector SpawnLocation = StaticMesh->GetComponentLocation()
		+ GetActorForwardVector() * 40.f;

	UE_LOG(LogShop, Warning, TEXT("[VM] Spawn Location = %s"), *SpawnLocation.ToString());

	FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);

	AAO_MasterItem* DropItem =
		GetWorld()->SpawnActorDeferred<AAO_MasterItem>(
			DroppableItemClass ? DroppableItemClass.Get() : AAO_MasterItem::StaticClass(),
			SpawnTransform,
			nullptr,
			nullptr,
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn
		);

	if (DropItem)
	{
		DropItem->ItemID = MechineItemID;
		UGameplayStatics::FinishSpawningActor(DropItem, SpawnTransform);
	}
}

void AAO_VendingMachine::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void AAO_VendingMachine::Server_RequestBuy_Implementation(AActor* Interactor)
{
	if (!HasAuthority())
		return;

	if (!ShopManager)
		return;

	UE_LOG(LogShop, Warning, TEXT("[VM] Server_RequestBuy"));

	ShopManager->Server_BuyItem(ItemPrice, this);
}

