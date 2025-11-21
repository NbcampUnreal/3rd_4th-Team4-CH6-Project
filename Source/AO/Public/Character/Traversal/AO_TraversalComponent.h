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
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

public:
	bool TryTraversal();

	bool GetDoingTraversal() const { return bDoingTraversal; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal")
	TObjectPtr<UChooserTable> AnimChooserTable;

	UPROPERTY(VisibleAnywhere, Replicated, Category = "Traversal")
	FTraversalCheckResult TraversalResult;
	FTraversalCheckInput TraversalInput;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal|Debug")
	int32 DrawDebugLevel = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal|Debug")
	float DrawDebugDuration = 5.f;

protected:
	UFUNCTION(Server, Reliable)
	void ServerRPC_PlayTraversal(const FTraversalCheckResult InTraversalResult);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPC_PlayTraversal(const FTraversalCheckResult InTraversalResult);
	
private:
	UPROPERTY()
	TObjectPtr<AActor> Owner = nullptr;
	UPROPERTY()
	TObjectPtr<ACharacter> Character = nullptr;
	UPROPERTY()
	TObjectPtr<UCharacterMovementComponent> CharacterMovement = nullptr;

	UPROPERTY(Replicated)
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
	void UpdateWarpTargets();
	
	UFUNCTION()
	void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
};
