// KSJ : AO_EQC_LastSeenLocation.h

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "AO_EQC_LastSeenLocation.generated.h"

/**
 * EQS Context : BB에서 'LastKnownPlayerLocation' 값 가져옴.
 * - 플레이어의 마지막 위치를 기준으로 회피 및 추격 점수 계산
 */
UCLASS()
class AO_API UAO_EQC_LastSeenLocation : public UEnvQueryContext
{
	GENERATED_BODY()

public:
	UAO_EQC_LastSeenLocation();

	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
};