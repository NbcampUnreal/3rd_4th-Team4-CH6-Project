// AO_AIC_Troll.h

#pragma once

#include "CoreMinimal.h"
#include "AI/Controller/AO_AIC_HumanoidBase.h"
#include "AO_AIC_Troll.generated.h"

class AAO_Enemy_Troll;

UCLASS()
class AO_API AAO_AIC_Troll : public AAO_AIC_HumanoidBase
{
	GENERATED_BODY()

public:
	AAO_AIC_Troll();

protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY()
	TObjectPtr<AAO_Enemy_Troll> ControlledTroll;
};