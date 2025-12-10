#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Burst.h"
#include "AO_GameplayCueNotify_Burst_Fuel.generated.h"

UCLASS()
class AO_API UAO_GameplayCueNotify_Burst_Fuel : public UGameplayCueNotify_Burst
{
	GENERATED_BODY()

public:
	virtual void HandleGameplayCue(
	AActor* Target,
	EGameplayCueEvent::Type EventType,
	const FGameplayCueParameters& Parameters
) override;

	UPROPERTY(EditDefaultsOnly, Category="FuelAdd")
	UParticleSystem* FuelParticles;

	UPROPERTY(EditDefaultsOnly, Category="FuelAdd")
	USoundBase* FuelSound;
	
	//virtual bool OnActive(...) const override;
	//virtual bool WhileActive(...) const override;
	//virtual bool OnRemove(...) const override;


};
