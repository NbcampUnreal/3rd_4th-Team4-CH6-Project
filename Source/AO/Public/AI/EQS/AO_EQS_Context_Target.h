// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "AO_EQS_Context_Target.generated.h"

/**
 * EQS Context: 추격 대상 (플레이어) 위치 반환
 */
UCLASS()
class AO_API UAO_EQS_Context_Target : public UEnvQueryContext
{
	GENERATED_BODY()
	
public:
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
};
