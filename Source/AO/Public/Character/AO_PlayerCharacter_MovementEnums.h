// MovementEnums.h

#pragma once

#include "CoreMinimal.h"
#include "AO_PlayerCharacter_MovementEnums.generated.h"

UENUM(BlueprintType)
enum class EGait : uint8
{
	Walk		UMETA(DisplayName = "Walk"),
	Run			UMETA(DisplayName = "Run"),
	Sprint		UMETA(DisplayName = "Sprint")
};

UENUM(BlueprintType)
enum class ECustomMovementMode : uint8
{
	OnGround	UMETA(DisplayName = "OnGround"),
	InAir		UMETA(DisplayName = "InAir")
};

UENUM(BlueprintType)
enum class EMovementState : uint8
{
	Idle		UMETA(DisplayName = "Idle"),
	Moving		UMETA(DisplayName = "Moving")
};

UENUM(BlueprintType)
enum class EStance : uint8
{
	Stand		UMETA(DisplayName = "Stand"),
	Crouch		UMETA(DisplayName = "Crouch")
};