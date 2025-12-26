#include "Train/AO_TrainWorldSubsystem.h"
#include "Train/AO_newTrain.h"

void UAO_TrainWorldSubsystem::RegisterTrain(AAO_newTrain* InTrain)
{
	if (!InTrain) return;

	CachedTrain = InTrain;

	UE_LOG(LogTemp, Warning, TEXT("Subsystem: Train Registered"));

	OnTrainRegistered.Broadcast(InTrain);
}


AAO_newTrain* UAO_TrainWorldSubsystem::GetTrain() const
{
	return CachedTrain.Get();
}
