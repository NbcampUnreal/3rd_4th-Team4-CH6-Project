// JSH: AO_GameMode_Rest.h

#pragma once

#include "CoreMinimal.h"
#include "Game/GameMode/AO_GameMode_InGameBase.h"
#include "AO_GameMode_Rest.generated.h"

UCLASS()
class AO_API AAO_GameMode_Rest : public AAO_GameMode_InGameBase
{
	GENERATED_BODY()

public:
	AAO_GameMode_Rest();

protected:
	virtual void BeginPlay() override;
};