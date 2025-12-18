// AO_CameraProfile.h

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "AO_CameraProfile.generated.h"

USTRUCT(BlueprintType)
struct FAO_CameraSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Priority = 0;

	// Spring Arm
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float TargetArmLength = 150.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector SocketOffset = FVector(0.f, 50.f, 50.f);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bEnableCameraLag = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CameraLagSpeed = 10.f;

	// Camera
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float FOV = 90.f;

	// Blend
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float BlendInTime = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float BlendOutTime = 0.25f;
};

UCLASS()
class AO_API UAO_CameraProfile : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FAO_CameraSettings> Profiles;

	const FAO_CameraSettings* FindByTag(const FGameplayTag& Tag) const;
};
