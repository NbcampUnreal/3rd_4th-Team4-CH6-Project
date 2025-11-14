// TraversableComponent.h - KH

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TraversableComponent.generated.h"


class USplineComponent;

// Hurdle: Traverse over a thin object and end on the ground at a similar level (Low fence)
// Vault: Traverse over a thin object and end in a falling state (Tall fence, or elevated obstacle with no floor on the other side)
// Mantle: Traverse up and onto an object without passing over it
UENUM(BlueprintType)
enum class ETraversalActionType : uint8
{
	None		UMETA(DisplayName = "None"),
	Hurdle		UMETA(DisplayName = "Hurdle"),	
	Vault		UMETA(DisplayName = "Vault"),
	Mantle		UMETA(DisplayName = "Mantle")
};

USTRUCT()
struct FTraversalCheckResult
{
	GENERATED_BODY()

public:
	// Traversal Action Type
	UPROPERTY(EditAnywhere, Category = "Traversal")
	ETraversalActionType ActionType = ETraversalActionType::None;

	// Front Ledge Information
	UPROPERTY(EditAnywhere, Category = "Traversal")
	bool bHasFrontLedge = false;
	UPROPERTY(EditAnywhere, Category = "Traversal")
	FVector FrontLedgeLocation;
	UPROPERTY(EditAnywhere, Category = "Traversal")
	FVector FrontLedgeNormal;

	// Back Ledge Information
	UPROPERTY(EditAnywhere, Category = "Traversal")
	bool bHasBackLedge = false;
	UPROPERTY(EditAnywhere, Category = "Traversal")
	FVector BackLedgeLocation;
	UPROPERTY(EditAnywhere, Category = "Traversal")
	FVector BackLedgeNormal;

	// Back Floor Information
	UPROPERTY(EditAnywhere, Category = "Traversal")
	bool bHasBackFloor = false;
	UPROPERTY(EditAnywhere, Category = "Traversal")
	FVector BackFloorLocation;

	// Obstacle Information
	UPROPERTY(EditAnywhere, Category = "Traversal")
	float ObstacleHeight = 0.f;
	UPROPERTY(EditAnywhere, Category = "Traversal")
	float ObstacleDepth = 0.f;
	UPROPERTY(EditAnywhere, Category = "Traversal")
	float BackLedgeHeight = 0.f;
	UPROPERTY(EditAnywhere, Category = "Traversal")
	UPrimitiveComponent* HitComponent;

	// Montage Information
	UPROPERTY(EditAnywhere, Category = "Traversal")
	UAnimMontage* ChosenMontage;
	UPROPERTY(EditAnywhere, Category = "Traversal")
	float StartTime;
	UPROPERTY(EditAnywhere, Category = "Traversal")
	float PlayRate;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AO_API UTraversableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTraversableComponent();
	
protected:
	virtual void OnRegister() override;

public:
	void GetLedgeTransforms(FVector& HitLocation, FVector& ActorLocation, FTraversalCheckResult& TraversalResult);
	
protected:
	UPROPERTY(BlueprintReadOnly, Category = "Traversal")
	TArray<USplineComponent*> Ledges;
	UPROPERTY(BlueprintReadOnly, Category = "Traversal")
	TMap<USplineComponent*, USplineComponent*> OppositeLedges;
	UPROPERTY(BlueprintReadOnly, Category = "Traversal")
	float MinLedgeWidth = 60.f;

private:
	void ScanSplines();
	USplineComponent* FindLedgeClosestToActor(FVector& ActorLocation);
};
