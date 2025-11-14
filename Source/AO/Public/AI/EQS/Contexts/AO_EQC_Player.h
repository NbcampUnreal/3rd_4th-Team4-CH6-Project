// AO_EQC_Player.h

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "AO_EQC_Player.generated.h"

UCLASS()
class AO_API UAO_EQC_Player : public UEnvQueryContext
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "EQS")
	FName PlayerActorBBKeyName = TEXT("PlayerActor");

protected:
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
};