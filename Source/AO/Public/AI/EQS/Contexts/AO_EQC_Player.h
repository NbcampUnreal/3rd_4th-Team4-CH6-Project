// KSJ : AO_EQC_Player.h

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "AO_EQC_Player.generated.h"

/**
 * EQS 컨텍스트: 월드 내의 모든 유효한 플레이어 캐릭터(Pawn) 반환
 */
UCLASS()
class AO_API UAO_EQC_Player : public UEnvQueryContext
{
	GENERATED_BODY()

public:
	UAO_EQC_Player();

	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
};
