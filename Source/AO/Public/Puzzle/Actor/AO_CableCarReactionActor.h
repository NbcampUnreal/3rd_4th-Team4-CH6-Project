// HSJ : AO_CableCarReactionActor.h
#pragma once

#include "CoreMinimal.h"
#include "Puzzle/Actor/AO_PuzzleReactionActor.h"
#include "Components/SplineComponent.h"
#include "AO_CableCarReactionActor.generated.h"

UCLASS()
class AO_API AAO_CableCarReactionActor : public AAO_PuzzleReactionActor
{
    GENERATED_BODY()

public:
    AAO_CableCarReactionActor();

    virtual void Tick(float DeltaTime) override;
    virtual void ActivateReaction() override;
    virtual void DeactivateReaction() override;

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    virtual void OnRep_IsActivated() override {}
    virtual void OnRep_TargetProgress() override {}

    UFUNCTION()
    void OnRep_MovementCommand();

private:
    void StartMovement(bool bForward);
    void StopMovement();
    float ApplyEaseInOutCurve(float T) const;

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    TObjectPtr<USplineComponent> SplinePath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CableCar", 
        meta=(ClampMin="0.01", UIMin="0.01", UIMax="1.0"))
    float MovementSpeed = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CableCar",
        meta=(ClampMin="1.0", UIMin="1.0", UIMax="5.0"))
    float EaseStrength = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CableCar")
    TObjectPtr<USoundBase> MovementStartSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CableCar")
    TObjectPtr<USoundBase> MovementStopSound;

protected:
    UPROPERTY(ReplicatedUsing=OnRep_MovementCommand, BlueprintReadOnly, Category="CableCar")
    uint8 MovementCommandCounter = 0;

    UPROPERTY(Replicated, BlueprintReadOnly, Category="CableCar")
    bool bTargetIsForward = true;

private:
    float CurrentAlpha = 0.0f;
    float TargetAlpha = 0.0f;
    bool bIsMovingForward = true;
    bool bIsMoving = false;
    
    TArray<FVector> SplineWorldPoints;
    
    uint8 LastProcessedCommand = 0;
};