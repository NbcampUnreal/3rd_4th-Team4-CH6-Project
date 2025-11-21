// AO_EQC_PlayerPredictedLocation.h

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "AO_EQC_PlayerPredictedLocation.generated.h"

UCLASS()
class AO_API UAO_EQC_PlayerPredictedLocation : public UEnvQueryContext
{
	GENERATED_BODY()

public:
	UAO_EQC_PlayerPredictedLocation();
	
	UPROPERTY(EditDefaultsOnly, Category = "EQS")
	FName PlayerActorBBKeyName;
	
	UPROPERTY(EditDefaultsOnly, Category = "EQS")
	float PredictDistance;
	
	UPROPERTY(EditDefaultsOnly, Category = "EQS")
	float FallbackDistance;

protected:
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance,
								FEnvQueryContextData& ContextData) const override;
};
