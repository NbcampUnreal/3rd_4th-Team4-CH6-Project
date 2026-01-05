#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AO_FuelData.generated.h"

UCLASS(BlueprintType)
class AO_API UAO_FuelData : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float InitialFuel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaximumFuel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RequireNextStage;
};
