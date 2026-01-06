#pragma once

#include "CoreMinimal.h"
#include "Interaction/Base/AO_BaseInteractable.h"
#include "Components/SphereComponent.h"
#include "AI/Item/AO_PickupComponent.h"
#include "AO_MasterItem.generated.h"

UCLASS()
class AO_API AAO_MasterItem : public AAO_BaseInteractable
{
	GENERATED_BODY()

public:
	AAO_MasterItem();

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	virtual FAO_InteractionInfo GetInteractionInfo(const FAO_InteractionQuery& InteractionQuery) const override;
	
public:
	virtual void OnInteractionSuccess_BP_Implementation(AActor* Interactor) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USphereComponent* InteractionSphere;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_ItemID, meta=(ExposeOnSpawn="true"))
	FName ItemID;

	UFUNCTION()
	void OnRep_ItemID();

	void ApplyItemData(); 

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category="Item")
	UDataTable* ItemDataTable;
	
	UPROPERTY(Replicated)
	float FuelAmount = 0.f;

	UPROPERTY(Replicated)
	FGameplayTagContainer ItemTags;

	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable)
	void Server_HandleInteraction(AActor* Interactor);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UAO_PickupComponent* PickupComponent;

	UPROPERTY(Replicated)
	FString CachedItemName;

};