// KSJ : AO_EQC_AllLastKnownLocations.h

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "AO_EQC_AllLastKnownLocations.generated.h"

/**
 * AI가 기억하고 있는 모든 적 플레이어의 마지막 위치들을 반환하는 컨텍스트
 * (멀티플레이어 환경에서 여러 명의 적 위치를 모두 회피하기 위함)
 */
UCLASS()
class AO_API UAO_EQC_AllLastKnownLocations : public UEnvQueryContext
{
	GENERATED_BODY()

public:
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
};
