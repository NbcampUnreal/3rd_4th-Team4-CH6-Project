// HSJ : AO_Interface_InspectionCameraTypes.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AO_Interface_InspectionCameraTypes.generated.h"

// 카메라 배치 모드
UENUM(BlueprintType)
enum class EInspectionCameraMode : uint8
{
	RelativeToActor    UMETA(DisplayName = "Relative To Actor"),
	WorldAbsolute      UMETA(DisplayName = "World Absolute")
};

// 카메라 이동 타입
UENUM(BlueprintType)
enum class EInspectionMovementType : uint8
{
	None        UMETA(DisplayName = "None"),
	Planar      UMETA(DisplayName = "Planar"),
	Orbital     UMETA(DisplayName = "Orbital"),
};

USTRUCT(BlueprintType)
struct FAO_InspectionCameraSettings
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	EInspectionCameraMode CameraMode = EInspectionCameraMode::RelativeToActor;

	UPROPERTY(BlueprintReadWrite)
	FVector CameraLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite)
	FRotator CameraRotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadWrite)
	EInspectionMovementType MovementType = EInspectionMovementType::None;

	UPROPERTY(BlueprintReadWrite)
	float MovementSpeed = 500.f;

	UPROPERTY(BlueprintReadWrite)
	FVector MovementBoundsExtent = FVector(500.f, 500.f, 0.f);
};

UINTERFACE(MinimalAPI, BlueprintType)
class UAO_Interface_InspectionCameraProvider : public UInterface
{
	GENERATED_BODY()
};

class AO_API IAO_Interface_InspectionCameraProvider
{
	GENERATED_BODY()

public:
	virtual FAO_InspectionCameraSettings GetInspectionCameraSettings() const = 0;
	virtual bool IsValidClickTarget(AActor* HitActor, UPrimitiveComponent* Component) const { return false; }
};