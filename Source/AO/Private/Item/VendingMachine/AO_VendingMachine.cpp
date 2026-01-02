#include "Item/VendingMachine/AO_VendingMachine.h"

#include "EngineUtils.h"
#include "Engine/DataTable.h"
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

	InteractionTitle = FText::FromString(TEXT("상품"));
	InteractionContent = FText::FromString(TEXT("구매"));
	
	PriceDisplayText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("PriceDisplay"));
	PriceDisplayText->SetupAttachment(RootComponent);
	PriceDisplayText->SetHorizontalAlignment(EHTA_Center);
	PriceDisplayText->SetVerticalAlignment(EVRTA_TextCenter);
	
	ItemExplainText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("ItemExplainDisplay"));
	ItemExplainText->SetupAttachment(RootComponent);
	ItemExplainText->SetHorizontalAlignment(EHTA_Center);
	ItemExplainText->SetVerticalAlignment(EVRTA_TextCenter);
}

void AAO_VendingMachine::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		if (!ShopManager)
		{
			for (TActorIterator<AAO_ShopManager> It(GetWorld()); It; ++It)
			{
				ShopManager = *It;
				break;
			}
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
				if (PriceDisplayText)
				{
					PriceDisplayText->SetText(FText::FromString(FString::Printf(TEXT("%d"), ItemPrice)));
				}
			}
			FString TableExplainData = Row->ItemExplain;
			if (!TableExplainData.IsEmpty())
			ItemExplainText->SetText(FText::FromString(FString::Printf(TEXT("%s"), *TableExplainData)));
		}
	}
}

void AAO_VendingMachine::OnInteractionSuccess(AActor* Interactor)
{
	Super::OnInteractionSuccess(Interactor);

	if (!HasAuthority())
	{
		return;
	}

	if (!ShopManager)
	{
		return;
	}
	
	ShopManager->Server_BuyItem(ItemPrice, this);
}


void AAO_VendingMachine::SpawnVendingItem()
{
	FVector SpawnLocation = StaticMesh->GetComponentLocation()
		+ GetActorForwardVector() * 100.f;

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
	
	ShopManager->Server_BuyItem(ItemPrice, this);
}

