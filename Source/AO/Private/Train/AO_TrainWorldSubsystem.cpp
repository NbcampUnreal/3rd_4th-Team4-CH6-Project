#include "Train/AO_TrainWorldSubsystem.h"
#include "AbilitySystemComponent.h"

void UAO_TrainWorldSubsystem::RegisterTrainASC(UAbilitySystemComponent* InASC)
{
	if (!InASC)
	{
		return;
	}

	CachedTrainASC = InASC;

	UE_LOG(LogTemp, Warning, TEXT("TrainWorldSubsystem: ASC Registered"));

	OnTrainASCRegistered.Broadcast(CachedTrainASC);
}

UAbilitySystemComponent* UAO_TrainWorldSubsystem::GetTrainASC() const
{
	return CachedTrainASC;
}

void UAO_TrainWorldSubsystem::BindAndNotifyTrainASC(UObject* Listener)
{
	if (!Listener)
	{
		return;
	}

	if (CachedTrainASC)
	{
		OnTrainASCRegistered.Broadcast(CachedTrainASC);
	}
}

void UAO_TrainWorldSubsystem::NotifyIfTrainASCReady()
{
	if (CachedTrainASC)
	{
		OnTrainASCRegistered.Broadcast(CachedTrainASC);
	}
}