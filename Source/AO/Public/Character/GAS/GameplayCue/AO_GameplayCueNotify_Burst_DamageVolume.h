// AO_GameplayCueNotify_Burst_DamageVolume.h

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Burst.h"
#include "AO_GameplayCueNotify_Burst_DamageVolume.generated.h"

UCLASS()
class AO_API UAO_GameplayCueNotify_Burst_DamageVolume : public UGameplayCueNotify_Burst
{
	GENERATED_BODY()

public:
	UAO_GameplayCueNotify_Burst_DamageVolume();

protected:
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	float MinInterval = 1.f;

	mutable double LastPlayTime = -1e9;

private:
	void PlayDamageReactSound(AActor* Target) const;
};
