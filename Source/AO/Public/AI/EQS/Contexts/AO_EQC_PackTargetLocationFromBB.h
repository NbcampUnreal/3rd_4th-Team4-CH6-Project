// AO_EQC_PackTargetLocationFromBB.h

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "AO_EQC_PackTargetLocationFromBB.generated.h"

UCLASS()
class AO_API UAO_EQC_PackTargetLocationFromBB : public UEnvQueryContext
{
	GENERATED_BODY()

public:
	UAO_EQC_PackTargetLocationFromBB();
	
	UPROPERTY(EditDefaultsOnly, Category = "EQS")
	FName PackTargetLocationBBKeyName;

protected:
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance,
								FEnvQueryContextData& ContextData) const override;
};
