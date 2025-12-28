// JSH: AO_GameMode_Rest.h

#pragma once

#include "CoreMinimal.h"
#include "Game/GameMode/AO_GameMode_InGameBase.h"
#include "Player/PlayerController/AO_PlayerController_Rest.h"
#include "AO_GameMode_Rest.generated.h"

UCLASS()
class AO_API AAO_GameMode_Rest : public AAO_GameMode_InGameBase
{
	GENERATED_BODY()

public:
	AAO_GameMode_Rest();

protected:
	virtual void BeginPlay() override;


public:
	// 휴게공간에서 "다음으로 진행" 상호작용 시 호출
	void HandleRestExitRequest(AController* Requester);
};