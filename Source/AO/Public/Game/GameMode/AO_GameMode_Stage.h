// AO_GameMode_Stage.h (장주만)

#pragma once

#include "CoreMinimal.h"
#include "AO_GameMode_InGameBase.h"
#include "AO_GameMode_Stage.generated.h"

/**
 * 
 */
UCLASS()
class AO_API AAO_GameMode_Stage : public AAO_GameMode_InGameBase
{
	GENERATED_BODY()

public:
	AAO_GameMode_Stage();

public:
	// 스테이지 출발 상호작용 시 서버에서 호출할 함수
	void HandleStageExitRequest(AController* Requester);

	// o 키 테스트용: 게임 실패 처리
	void HandleStageFail(AController* Requester);
};
