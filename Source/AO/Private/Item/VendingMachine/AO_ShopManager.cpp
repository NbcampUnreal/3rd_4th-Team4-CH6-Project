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
		// GI를 불러와서 '현재' 저장된 연료 값을 그대로 가져옵니다.
		if (UAO_GameInstance* GI = Cast<UAO_GameInstance>(GetGameInstance()))
		{
			// ResetRun에서 세팅된 80이든, 이전 스테이지에서 쓰고 남은 50이든 그대로 가져옴
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
    
	// 1. 매니저의 돈 차감
	SharedShopMoney -= Cost;
    
	// 2. 중요: GameInstance의 값도 즉시 업데이트 (심리스 트래블 시 이 값을 들고 감)
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
	if (UAO_GameInstance* MyGI = Cast<UAO_GameInstance>(GI))
	{
		// 'GetInitialFuelValue(80.0)'가 아니라, 
		// 현재 GI에 남아있는 'SharedTrainFuel'을 가져와야 유지됩니다!
		SharedShopMoney = FMath::FloorToInt(MyGI->SharedTrainFuel);
	}
}