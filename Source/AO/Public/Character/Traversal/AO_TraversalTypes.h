// AO_TraversalTypes.h - KH

#pragma once

#include "CoreMinimal.h"
#include "Character/AO_PlayerCharacter_MovementEnums.h"
#include "AO_TraversalTypes.generated.h"

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

// Has all the information needed to perform traversal action
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
	FVector FrontLedgeLocation = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, Category = "Traversal")
	FVector FrontLedgeNormal = FVector::ZeroVector;

	// Back Ledge Information
	UPROPERTY(EditAnywhere, Category = "Traversal")
	bool bHasBackLedge = false;
	UPROPERTY(EditAnywhere, Category = "Traversal")
	FVector BackLedgeLocation = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, Category = "Traversal")
	FVector BackLedgeNormal = FVector::ZeroVector;

	// Back Floor Information
	UPROPERTY(EditAnywhere, Category = "Traversal")
	bool bHasBackFloor = false;
	UPROPERTY(EditAnywhere, Category = "Traversal")
	FVector BackFloorLocation = FVector::ZeroVector;

	// Obstacle Information
	UPROPERTY(EditAnywhere, Category = "Traversal")
	float ObstacleHeight = 0.f;
	UPROPERTY(EditAnywhere, Category = "Traversal")
	float ObstacleDepth = 0.f;
	UPROPERTY(EditAnywhere, Category = "Traversal")
	float BackLedgeHeight = 0.f;
	UPROPERTY(EditAnywhere, Category = "Traversal")
	TObjectPtr<UPrimitiveComponent> HitComponent = nullptr;

	// Montage Information
	UPROPERTY(EditAnywhere, Category = "Traversal")
	TObjectPtr<UAnimMontage> ChosenMontage = nullptr;
	UPROPERTY(EditAnywhere, Category = "Traversal")
	float StartTime = 0.f;
	UPROPERTY(EditAnywhere, Category = "Traversal")
	float PlayRate = 0.f;
};

// Input Structure For Evaluating Traversal Montage's Chooser Table
USTRUCT(BlueprintType)
struct FTraversalChooserInput
{
	GENERATED_BODY()

public:
	// Traversal Action Type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal", meta = (BlueprintThreadSafe))
	ETraversalActionType ActionType = ETraversalActionType::None;

	// For Selecting Traversal Action Type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal", meta = (BlueprintThreadSafe))
	bool bHasFrontLedge = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal", meta = (BlueprintThreadSafe))
	bool bHasBackLedge = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal", meta = (BlueprintThreadSafe))
	bool bHasBackFloor = false;

	// Obstacle Information
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal", meta = (BlueprintThreadSafe))
	float ObstacleHeight = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal", meta = (BlueprintThreadSafe))
	float ObstacleDepth = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal", meta = (BlueprintThreadSafe))
	float BackLedgeHeight = 0.f;

	// Character Movement Information
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal", meta = (BlueprintThreadSafe))
	TEnumAsByte<EMovementMode> MovementMode = MOVE_None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal", meta = (BlueprintThreadSafe))
	EGait Gait = EGait::Run;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal", meta = (BlueprintThreadSafe))
	float Speed2D = 0.f;
};

// Output Structure For Evaluating Traversal Montage's Chooser Table
USTRUCT(BlueprintType)
struct FTraversalChooserOutput
{
	GENERATED_BODY()

public:
	// Traversal Action Type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal", meta = (BlueprintThreadSafe))
	ETraversalActionType ActionType = ETraversalActionType::None;
};

// Input Structure For Tracing Traversable Objects
USTRUCT()
struct FTraversalCheckInput
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Traversal")
	FVector TraceForwardDirection = FVector::ForwardVector;
	UPROPERTY(EditAnywhere, Category = "Traversal")
	float TraceForwardDistance = 75.f;
	UPROPERTY(EditAnywhere, Category = "Traversal")
	FVector TraceOriginOffset = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, Category = "Traversal")
	FVector TraceEndOffset = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, Category = "Traversal")
	float TraceRadius = 30.f;
	UPROPERTY(EditAnywhere, Category = "Traversal")
	float TraceHalfHeight = 86.f;
};