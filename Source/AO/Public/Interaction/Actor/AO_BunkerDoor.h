// HSJ : AO_BunkerDoor.h
#pragma once

#include "CoreMinimal.h"
#include "Interaction/Base/AO_BaseInteractable.h"
#include "AO_BunkerDoor.generated.h"

UCLASS()
class AO_API AAO_BunkerDoor : public AAO_BaseInteractable
{
	GENERATED_BODY()

public:
	AAO_BunkerDoor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual bool CanInteraction(const FAO_InteractionQuery& InteractionQuery) const override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnInteractionSuccess_BP_Implementation(AActor* Interactor) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_DoorState();

private:
	void OpenDoor();
	void CloseDoor();
	void CheckDoorAnimation();

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USkeletalMeshComponent> DoorSkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BunkerDoor")
	TObjectPtr<UAnimationAsset> DoorAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BunkerDoor",
		meta=(ClampMin="0.1", UIMin="0.1", UIMax="5.0"))
	float DoorAnimationSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BunkerDoor")
	TObjectPtr<USoundBase> DoorOpenSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BunkerDoor")
	TObjectPtr<USoundBase> DoorCloseSound;

protected:
	UPROPERTY(ReplicatedUsing=OnRep_DoorState, BlueprintReadOnly, Category="BunkerDoor")
	bool bDoorOpen = false;

	UPROPERTY(Replicated, BlueprintReadOnly, Category="BunkerDoor")
	bool bIsAnimating = false;

private:
	FTimerHandle DoorAnimationCheckTimer;
};