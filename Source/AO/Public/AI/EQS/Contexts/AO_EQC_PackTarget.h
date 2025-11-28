// KSJ : AO_EQC_PackTarget.h

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "AO_EQC_PackTarget.generated.h"

/**
 * EQS 쿼리용 컨텍스트
 * PackManager가 이 늑대에게 할당한 "포위 타겟(플레이어)"을 반환
 */
UCLASS()
class AO_API UAO_EQC_PackTarget : public UEnvQueryContext
{
	GENERATED_BODY()

public:
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
};

