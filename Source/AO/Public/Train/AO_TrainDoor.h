#pragma once

#include "CoreMinimal.h"
#include "Interaction/Base/AO_BaseInteractable.h"
#include "AO_TrainDoor.generated.h"

UCLASS()
class AO_API AAO_TrainDoor : public AAO_BaseInteractable
{
	GENERATED_BODY()

public:
	AAO_TrainDoor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual bool CanInteraction(const FAO_InteractionQuery& InteractionQuery) const override;
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	virtual void OnInteractionSuccess_BP_Implementation(AActor* Interactor) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_DoorState();

private:
	void PlayDoorSound();

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> DoorMesh;

	//거리
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TrainDoor")
	FVector SlideOffset = FVector(-100.0f, 0.0f, 0.0f);

	//속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TrainDoor")
	float SlideSpeed = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TrainDoor")
	TObjectPtr<USoundBase> DoorOpenSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TrainDoor")
	TObjectPtr<USoundBase> DoorCloseSound;
    
	UPROPERTY(ReplicatedUsing=OnRep_DoorState, BlueprintReadOnly, Category="TrainDoor")
	bool bDoorOpen = false;

private:
	FVector ClosedLocation;
	FVector OpenedLocation;
};