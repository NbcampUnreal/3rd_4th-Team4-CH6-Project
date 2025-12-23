#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ITrainFuelListener.generated.h"

UINTERFACE(BlueprintType)
class UTrainFuelListener : public UInterface
{
	GENERATED_BODY()
};

class ITrainFuelListener
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnFuelChanged(float NewFuel);
};