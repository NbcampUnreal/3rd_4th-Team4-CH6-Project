// AO_EQS_Context_AllPlayers.h

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "AO_EQS_Context_AllPlayers.generated.h"

/**
 * 월드 내 모든 플레이어 캐릭터의 위치를 반환하는 EQS Context
 * Insect가 "다른 플레이어들로부터 먼 곳"을 찾을 때 사용
 */
UCLASS()
class AO_API UAO_EQS_Context_AllPlayers : public UEnvQueryContext
{
	GENERATED_BODY()

public:
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
};

