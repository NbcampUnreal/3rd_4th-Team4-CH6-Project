#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "AO_TrainWorldSubsystem.generated.h"

class AAO_newTrain;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnTrainRegistered,
	AAO_newTrain*, Train
);

UCLASS()
class UAO_TrainWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	void RegisterTrain(AAO_newTrain* InTrain);

	UFUNCTION(BlueprintCallable)
	AAO_newTrain* GetTrain() const;

	UPROPERTY(BlueprintAssignable)
	FOnTrainRegistered OnTrainRegistered;

private:
	UPROPERTY()
	TObjectPtr<AAO_newTrain> CachedTrain;
};
