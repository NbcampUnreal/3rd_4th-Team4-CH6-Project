// base interect item 상속. add fuel기능과 crab 상호작용을 component화 시켜서 적용해보려함

// this item can.
// 1) interection with Press F
// 2) Can Be Fuel
// 3) Crab monster can bring it //to do 

#pragma once

#include "CoreMinimal.h"
#include "Interaction/Base/AO_BaseInteractable.h"
#include "Components/SphereComponent.h"
#include "Item/AO_ItemPickupComponent.h"
#include "Item/AO_ItemFuelComponent.h"
#include "AO_MasterItem.generated.h"

UCLASS()
class AO_API AAO_MasterItem : public AAO_BaseInteractable
{
	GENERATED_BODY()

public:
	AAO_MasterItem();

protected:
	virtual void BeginPlay() override;

public:
	virtual void OnInteractionSuccess_BP_Implementation(AActor* Interactor) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USphereComponent* InteractionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UAO_ItemPickupComponent* PickupComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UAO_ItemFuelComponent* FuelComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	UDataTable* ItemDataTable;

	// GameplayTags
	UPROPERTY(Replicated)
	FGameplayTagContainer ItemTags;

	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};