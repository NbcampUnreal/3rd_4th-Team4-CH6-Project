#include "Item/VendingMachine/AO_ShopManager.h"
#include "Game/GameInstance/AO_GameInstance.h"
#include "Item/VendingMachine/AO_VendingMachine.h"

AAO_ShopManager::AAO_ShopManager()
{
	bReplicates = true;
	bAlwaysRelevant = true; 
	PrimaryActorTick.bCanEverTick = false;
	
	MoneyDisplayText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("MoneyDisplay"));
	MoneyDisplayText->SetupAttachment(RootComponent);
	MoneyDisplayText->SetHorizontalAlignment(EHTA_Center);
	MoneyDisplayText->SetVerticalAlignment(EVRTA_TextCenter);
	MoneyDisplayText->SetWorldSize(70.0f);
	MoneyDisplayText->SetTextRenderColor(FColor::Yellow);
}

void AAO_ShopManager::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

#if WITH_EDITOR
	if (!HasAuthority())
	{
		return;
	}
	UpdateMoneyDisplay();
#endif
}

void AAO_ShopManager::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		InitializeFromGI(GetGameInstance());
		ForceNetUpdate();
	}

	UpdateMoneyDisplay();
}

void AAO_ShopManager::OnRep_TotalMoney()
{
	UpdateMoneyDisplay();
}


void AAO_ShopManager::Server_BuyItem_Implementation(int32 Cost, AAO_VendingMachine* Vendor)
{
	if (!HasAuthority())
		return;

	if (!Vendor)
		return;

	if (SharedShopMoney < Cost)
		return;
	
	SharedShopMoney -= Cost;
	
	UpdateMoneyDisplay();

	ForceNetUpdate();

	Vendor->SpawnVendingItem();
}


void AAO_ShopManager::UpdateMoneyDisplay()
{
	if (MoneyDisplayText)
	{
		MoneyDisplayText->SetText(FText::FromString(FString::Printf(TEXT("Shop Money: %d"), SharedShopMoney)));
	}
}

void AAO_ShopManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAO_ShopManager, SharedShopMoney);
}

void AAO_ShopManager::InitializeFromGI(UGameInstance* GI)
{
	if (!HasAuthority()) return;

	UWorld* World = GetWorld();
	if (!World) return;
	
	if (UAO_GameInstance* GI = Cast<UAO_GameInstance>(World->GetGameInstance()))
	{
		SharedShopMoney = FMath::Max(0.0f, GI->SharedTrainFuel);
	}
}