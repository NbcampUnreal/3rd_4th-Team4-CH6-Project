#pragma once

#include "CoreMinimal.h"
#include "AO_ShopManager.h"
#include "GameFramework/Actor.h"
#include "Item/AO_MasterItem.h"
#include "AO_VendingMachine.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(LogShop, Log, All);

UCLASS()
class AO_API AAO_VendingMachine : public AActor
{
	GENERATED_BODY()

public:
	AAO_VendingMachine();

protected:
	UPROPERTY(EditInstanceOnly, Category="Shop")
	AAO_ShopManager* ShopManager;
	
	virtual void BeginPlay() override;
	void OnConstruction(const FTransform& Transform);
	
	UFUNCTION()
	void OnRep_ItemID();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
	TObjectPtr<UStaticMeshComponent> StaticMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
	TObjectPtr<UStaticMeshComponent> ItemMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interact")
	class UAO_InteractableComponent* InteractableComp;

	UPROPERTY(EditDefaultsOnly, Category="Inventory") 
	TSubclassOf<AAO_MasterItem> DroppableItemClass;
	
	void ApplyItemData();
	UFUNCTION()
	void HandleInteractionSuccess(AActor* Interactor);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable)
	void Server_RequestBuy(AActor* Interactor);

	
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category="Item")
	UDataTable* ItemDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_ItemID, meta=(ExposeOnSpawn="true"))
	FName MechineItemID;
	
	int32 Cash;
	int32 ItemPrice;

public:
	void SpawnVendingItem();
};
