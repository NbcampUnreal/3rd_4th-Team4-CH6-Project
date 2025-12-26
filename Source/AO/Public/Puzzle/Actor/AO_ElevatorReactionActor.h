// HSJ : AO_ElevatorReactionActor.h
#pragma once

#include "CoreMinimal.h"
#include "Puzzle/Actor/AO_PuzzleReactionActor.h"
#include "AO_ElevatorReactionActor.generated.h"

UENUM(BlueprintType)
enum class EElevatorStep : uint8
{
    OpenDoor,
    WaitDoor,
    CloseDoor,
    MoveToGround,
    MoveToBasement,
    Complete
};

UCLASS()
class AO_API AAO_ElevatorReactionActor : public AAO_PuzzleReactionActor
{
    GENERATED_BODY()

public:
    AAO_ElevatorReactionActor();

    virtual void Tick(float DeltaTime) override;
    
    UFUNCTION(BlueprintCallable, Category="Elevator")
    void CallFromGround();
    
    UFUNCTION(BlueprintCallable, Category="Elevator")
    void CallFromBasement();
    
    UFUNCTION(BlueprintCallable, Category="Elevator")
    void CallFromInterior();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    virtual void ActivateReaction() override {}
    virtual void DeactivateReaction() override {}
    virtual void OnRep_IsActivated() override {}
    virtual void OnRep_TargetProgress() override {}

    UFUNCTION()
    void OnRep_ElevatorSequence();

private:
    void BuildSequenceForGroundButton();
    void BuildSequenceForBasementButton();
    void BuildSequenceForInteriorButton();
    
    void ExecuteNextStep();
    void ExecuteStep(EElevatorStep Step);
    void OnStepComplete();
    
    void OpenDoor();
    void CloseDoor();
    void StartDoorTimer();
    void OnDoorTimerComplete();
    void MoveToGround();
    void MoveToBasement();
    
    void UpdateElevatorMovement(float DeltaTime);
    void UpdateDoorMovement(float DeltaTime);
    float ApplyEaseInOutCurve(float T) const;

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    TObjectPtr<UStaticMeshComponent> LeftDoorMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    TObjectPtr<UStaticMeshComponent> RightDoorMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Elevator",
        meta=(ClampMin="0.1", UIMin="0.1", UIMax="20.0"))
    float ElevatorSpeed = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Elevator",
        meta=(ClampMin="1.0", UIMin="1.0", UIMax="5.0"))
    float EaseStrength = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Elevator",
        meta=(ClampMin="0.5", UIMin="0.5", UIMax="10.0"))
    float DoorOpenDuration = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Elevator",
        meta=(ClampMin="50.0", UIMin="50.0", UIMax="300.0"))
    float DoorOpenDistance = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Elevator",
        meta=(ClampMin="1.0", UIMin="1.0", UIMax="10.0"))
    float DoorSpeed = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Elevator")
    TObjectPtr<USoundBase> DoorOpenSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Elevator")
    TObjectPtr<USoundBase> DoorCloseSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Elevator")
    TObjectPtr<USoundBase> MovementStartSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Elevator")
    TObjectPtr<USoundBase> MovementStopSound;

protected:
    UPROPERTY(ReplicatedUsing=OnRep_ElevatorSequence, BlueprintReadOnly, Category="Elevator")
    uint8 SequenceCounter = 0;

    UPROPERTY(Replicated, BlueprintReadOnly, Category="Elevator")
    TArray<EElevatorStep> ReplicatedStepQueue;

private:
    FVector InitialLocation;
    FRotator InitialRotation;
    
    FVector InitialActorLocation;
    FRotator InitialActorRotation;
    
    FVector LeftDoorInitialLocation;
    FVector RightDoorInitialLocation;
    
    float CurrentProgress = 0.0f;
    float TargetProgressValue = 0.0f;
    
    float CurrentDoorProgress = 0.0f;
    float TargetDoorProgress = 0.0f;
    
    bool bIsProcessing = false;
    bool bIsMoving = false;
    bool bIsDoorMoving = false;
    
    TArray<EElevatorStep> StepQueue;
    int32 CurrentStepIndex = 0;
    
    FTimerHandle DoorTimerHandle;
    
    uint8 LastProcessedSequence = 0;
};