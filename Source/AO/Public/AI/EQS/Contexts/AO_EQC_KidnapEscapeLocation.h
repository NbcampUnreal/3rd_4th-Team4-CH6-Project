// KSJ : AO_EQC_KidnapEscapeLocation.h

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "AO_EQC_KidnapEscapeLocation.generated.h"

/**
 * EQS Context: 납치 도주 위치 탐색
 * - 납치 시작 지점에서 멀리
 * - 다른 플레이어와 가깝지 않은 위치
 */
UCLASS()
class AO_API UAO_EQC_KidnapEscapeLocation : public UEnvQueryContext
{
	GENERATED_BODY()

public:
	UAO_EQC_KidnapEscapeLocation();

	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
};
