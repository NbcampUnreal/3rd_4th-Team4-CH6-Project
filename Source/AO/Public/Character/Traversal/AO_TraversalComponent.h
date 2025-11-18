// AO_TraversalComponent.h - KH

#pragma once

#include "CoreMinimal.h"
#include "AO_TraversalTypes.h"
#include "Chooser.h"
#include "Components/ActorComponent.h"
#include "AO_TraversalComponent.generated.h"

class UCharacterMovementComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AO_API UAO_TraversalComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAO_TraversalComponent();

protected:
	virtual void BeginPlay() override;

public:
	bool TryTraversal();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal")
	TObjectPtr<UChooserTable> AnimChooserTable;
	
	FTraversalCheckResult TraversalResult;
	FTraversalCheckInput TraversalInput;
	int32 DrawDebugLevel = 2;
	float DrawDebugDuration = 20.f;
	
private:
	UPROPERTY()
	TObjectPtr<AActor> Owner = nullptr;
	UPROPERTY()
	TObjectPtr<ACharacter> Character = nullptr;
	UPROPERTY()
	TObjectPtr<UCharacterMovementComponent> CharacterMovement = nullptr;
	
	bool bDoingTraversal = false;

private:
	bool GetTraversalCheckInputs();
	bool RunCapsuleTrace(
		const FVector& StartLocation,
		const FVector& EndLocation,
		float Radius,
		float HalfHeight,
		FHitResult& OutHit,
		FColor DebugHitColor,
		FColor DebugTraceColor);
	void PerformTraversalAnimation();
	
	UFUNCTION()
	void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
};
