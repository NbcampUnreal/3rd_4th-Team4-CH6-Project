// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "AO_EQS_Context_PackMembers.generated.h"

/**
 * EQS Context: 주변 Werewolf 무리 멤버들 반환 (자신 제외)
 * - 포위 시 겹치지 않게 하기 위함
 */
UCLASS()
class AO_API UAO_EQS_Context_PackMembers : public UEnvQueryContext
{
	GENERATED_BODY()
	
public:
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
};
