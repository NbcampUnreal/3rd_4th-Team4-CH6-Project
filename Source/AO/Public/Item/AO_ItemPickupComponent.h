#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "GameplayTagAssetInterface.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "AO_ItemPickupComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AO_API UAO_ItemPickupComponent : public UActorComponent, public IGameplayTagAssetInterface
{
    GENERATED_BODY()

public:
    UAO_ItemPickupComponent();

protected:
    virtual void BeginPlay() override;

    // ---------------- Visual Helpers ----------------
    void ApplyPickedUpVisuals();
    void ApplyDroppedVisuals();

    UFUNCTION()
    void OnRep_Holder();

    UFUNCTION()
    void OnRep_IsPickedUp();

public:
    // ---------------- Pickup / Drop ----------------
    UFUNCTION(BlueprintCallable, Category="Pickup")
    void Pickup(USceneComponent* AttachTo, FName SocketName);

    UFUNCTION(BlueprintCallable, Category="Pickup")
    void Drop();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    USphereComponent* SphereComponent;

    UPROPERTY(ReplicatedUsing=OnRep_IsPickedUp, VisibleAnywhere, BlueprintReadOnly)
    bool bIsPickedUp = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bDropWithPhysics = false;

    UPROPERTY(ReplicatedUsing=OnRep_Holder, VisibleAnywhere, BlueprintReadOnly)
    AActor* HolderActor = nullptr;

    // ---------------- Gameplay Tags ----------------
    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
    FGameplayTagContainer ItemTags;

    UFUNCTION(BlueprintCallable, Category="Tags")
    void AddGameplayTag(FGameplayTag Tag);

    UFUNCTION(BlueprintCallable, Category="Tags")
    void RemoveGameplayTag(FGameplayTag Tag);

    virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
