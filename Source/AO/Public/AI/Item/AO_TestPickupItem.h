// AO_TestPickupItem.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "GameplayTagAssetInterface.h"
#include "AO_TestPickupItem.generated.h"

UCLASS()
class AO_API AAO_TestPickupItem : public AActor, public IGameplayTagAssetInterface
{
	GENERATED_BODY()
	
public:
	AAO_TestPickupItem();
	
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_Holder();

	UFUNCTION()
	void OnRep_IsPickedUp();

	void ApplyPickedUpVisuals();
	void ApplyDroppedVisuals();

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup")
	class UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup")
	class USphereComponent* SphereComponent;

	UPROPERTY(ReplicatedUsing = OnRep_IsPickedUp, VisibleAnywhere, BlueprintReadOnly, Category = "Pickup")
	bool bIsPickedUp;

	UPROPERTY(EditAnywhere, Category = "Pickup")
	bool bDropWithPhysics = false;

	UPROPERTY(ReplicatedUsing = OnRep_Holder)
	AActor* HolderActor = nullptr;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Tags")
	FGameplayTagContainer ItemTags;

	UFUNCTION(BlueprintCallable, Category = "Tags")
	void AddGameplayTag(FGameplayTag Tag);

	UFUNCTION(BlueprintCallable, Category = "Tags")
	void RemoveGameplayTag(FGameplayTag Tag);

	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;

	UFUNCTION(BlueprintCallable, Category = "Pickup")
	void Pickup(USceneComponent* AttachTo, FName SocketName);

	UFUNCTION(BlueprintCallable, Category = "Pickup")
	void Drop();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};