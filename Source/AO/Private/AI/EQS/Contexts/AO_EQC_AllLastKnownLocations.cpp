// KSJ : AO_EQC_AllLastKnownLocations.cpp

#include "AI/EQS/Contexts/AO_EQC_AllLastKnownLocations.h"
#include "AI/Controller/AO_AIC_Crab.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Point.h"

void UAO_EQC_AllLastKnownLocations::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	AActor* QueryOwner = Cast<AActor>(QueryInstance.Owner.Get());
	AAO_AIC_Crab* AIC = nullptr;

	if (AAO_AIC_Crab* Controller = Cast<AAO_AIC_Crab>(QueryOwner))
	{
		AIC = Controller;
	}
	else if (APawn* Pawn = Cast<APawn>(QueryOwner))
	{
		AIC = Cast<AAO_AIC_Crab>(Pawn->GetController());
	}

	if (AIC)
	{
		TArray<FVector> Locations = AIC->GetAllLastKnownLocations();
		if (Locations.Num() > 0)
		{
			UEnvQueryItemType_Point::SetContextHelper(ContextData, Locations);
		}
	}
}
