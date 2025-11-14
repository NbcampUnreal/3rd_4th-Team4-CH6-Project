// AO_EQC_BlackboardLocation.h

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "AO_EQC_BlackboardLocation.generated.h"

UCLASS()
class AO_API UAO_EQC_BlackboardLocation : public UEnvQueryContext
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "EQS")
	FName LocationBBKeyName = TEXT("MoveToLocation");

protected:
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance,
								FEnvQueryContextData& ContextData) const override;
};
