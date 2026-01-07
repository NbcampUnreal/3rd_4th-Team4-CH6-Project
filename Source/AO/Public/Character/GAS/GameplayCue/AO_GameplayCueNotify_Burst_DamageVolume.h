// AO_GameplayCueNotify_Burst_DamageVolume.h

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Burst.h"
#include "AO_GameplayCueNotify_Burst_DamageVolume.generated.h"

UCLASS()
class AO_API UAO_GameplayCueNotify_Burst_DamageVolume : public UGameplayCueNotify_Burst
{
	GENERATED_BODY()

protected:
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;

private:
	void PlayDamageReactSound(AActor* Target) const;
};
