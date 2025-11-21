// AO_EQC_LastSeenLocation.h

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "AO_EQC_LastSeenLocation.generated.h"

UCLASS()
class AO_API UAO_EQC_LastSeenLocation : public UEnvQueryContext
{
	GENERATED_BODY()

protected:
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance,
								FEnvQueryContextData& ContextData) const override;
};
