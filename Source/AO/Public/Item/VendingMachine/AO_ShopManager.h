#pragma once

#include "CoreMinimal.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"
#include "AO_ShopManager.generated.h"

class AAO_VendingMachine;

UCLASS()
class AO_API AAO_ShopManager : public AActor
{
	GENERATED_BODY()

public:
	AAO_ShopManager();
	void OnConstruction(const FTransform& Transform);

	UPROPERTY(ReplicatedUsing=OnRep_TotalMoney)
	int32 SharedShopMoney;

	UPROPERTY(VisibleAnywhere)
	UTextRenderComponent* MoneyDisplayText;

	UFUNCTION()
	void OnRep_TotalMoney();
	UFUNCTION(Server, Reliable)
	void Server_BuyItem(int32 Cost, AAO_VendingMachine* Vendor);
	
	void UpdateMoneyDisplay();


protected:
	virtual void BeginPlay() override;
	void InitializeFromGI(UGameInstance* GI);
	
};
