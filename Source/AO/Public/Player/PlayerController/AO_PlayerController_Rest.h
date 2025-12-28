// JSH : AO_PlayerController_Rest.h
#pragma once

#include "CoreMinimal.h"
#include "Player/PlayerController/AO_PlayerController_InGameBase.h"
#include "AO_PlayerController_Rest.generated.h"

UCLASS()
class AO_API AAO_PlayerController_Rest : public AAO_PlayerController_InGameBase
{
	GENERATED_BODY()

public:
	UFUNCTION(Server, Reliable)
	void Server_RequestRestExit();
};
