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
		if (UAO_GameInstance* GI = Cast<UAO_GameInstance>(GetGameInstance()))
		{
			SharedShopMoney = FMath::FloorToInt(GI->SharedTrainFuel);
		}
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
	if (!HasAuthority() || !Vendor || SharedShopMoney < Cost) return;
	
	SharedShopMoney -= Cost;
	
	if (UAO_GameInstance* GI = Cast<UAO_GameInstance>(GetGameInstance()))
	{
		GI->SharedTrainFuel = (float)SharedShopMoney;
	}
    
	UpdateMoneyDisplay();
	Vendor->SpawnVendingItem();
}

void AAO_ShopManager::UpdateMoneyDisplay()
{
	if (MoneyDisplayText)
	{
		MoneyDisplayText->SetText(FText::FromString(FString::Printf(TEXT("Fuel Remaining: %d"), SharedShopMoney)));	// JM : Shop Money -> Fuel 으로 변경
	}
}

void AAO_ShopManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAO_ShopManager, SharedShopMoney);
}

void AAO_ShopManager::InitializeFromGI(UGameInstance* GI)
{
	if (UAO_GameInstance* MyGI = Cast<UAO_GameInstance>(GI))
	{
		SharedShopMoney = FMath::FloorToInt(MyGI->SharedTrainFuel);
	}
}