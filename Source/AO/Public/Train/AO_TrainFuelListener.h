#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AO_TrainFuelListener.generated.h"

UINTERFACE(BlueprintType)
class UAO_TrainFuelListener : public UInterface
{
	GENERATED_BODY()
};

class IAO_TrainFuelListener
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnFuelChanged(float NewFuel);
};