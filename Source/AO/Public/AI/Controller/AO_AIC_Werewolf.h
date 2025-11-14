// AO_AIC_Werewolf.h

#pragma once

#include "CoreMinimal.h"
#include "AI/Controller/AO_AIC_HumanoidBase.h"
#include "AO_AIC_Werewolf.generated.h"

class AAO_Enemy_Werewolf;

UCLASS()
class AO_API AAO_AIC_Werewolf : public AAO_AIC_HumanoidBase
{
	GENERATED_BODY()

public:
	AAO_AIC_Werewolf();

protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY()
	TObjectPtr<AAO_Enemy_Werewolf> ControlledWerewolf;

public:
	void HandleHowlEvent(const FVector& HowlLocation);
};
