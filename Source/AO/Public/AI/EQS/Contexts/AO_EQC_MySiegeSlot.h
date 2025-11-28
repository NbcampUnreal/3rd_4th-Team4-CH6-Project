// KSJ : AO_EQC_MySiegeSlot.h

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "AO_EQC_MySiegeSlot.generated.h"

/**
 * EQS 쿼리용 컨텍스트
 * PackManager가 이 늑대에게 할당한 "슬롯 방향"을 반환
 * (도주로 차단 위치 계산용)
 */
UCLASS()
class AO_API UAO_EQC_MySiegeSlot : public UEnvQueryContext
{
	GENERATED_BODY()

public:
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
};

