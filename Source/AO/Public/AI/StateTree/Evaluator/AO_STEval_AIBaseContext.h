//KSJ : AO_STEval_AIBaseContext

#pragma once

#include "CoreMinimal.h"
#include "StateTreeEvaluatorBase.h"
#include "AO_STEval_AIBaseContext.generated.h"

class AAO_AICharacterBase;
class AAO_AIControllerBase;
class AAO_PlayerCharacter;

/**
 * AI 기본 상태 평가 Evaluator 인스턴스 데이터
 * 모든 AI가 공통으로 사용하는 기본 상태 정보
 */
USTRUCT()
struct FAO_STEval_AIBaseContext_InstanceData
{
	GENERATED_BODY()

	// 시야 내 플레이어 존재 여부
	UPROPERTY(EditAnywhere, Category = "Output")
	bool bHasPlayerInSight = false;

	// 기절 상태 여부
	UPROPERTY(EditAnywhere, Category = "Output")
	bool bIsStunned = false;

	// 가장 가까운 플레이어까지의 거리
	UPROPERTY(EditAnywhere, Category = "Output")
	float NearestPlayerDistance = MAX_FLT;

	// 가장 가까운 플레이어
	UPROPERTY(EditAnywhere, Category = "Output")
	TWeakObjectPtr<AAO_PlayerCharacter> NearestPlayer;

	// 소리로 감지된 위치 (있으면)
	UPROPERTY(EditAnywhere, Category = "Output")
	FVector LastHeardLocation = FVector::ZeroVector;

	// 소리 감지 여부
	UPROPERTY(EditAnywhere, Category = "Output")
	bool bHasHeardSound = false;
};

/**
 * AI 기본 상태 평가 Evaluator
 * - 모든 AI가 공통으로 사용하는 기본 상태를 평가
 * - 자식 Evaluator는 이 Evaluator의 데이터를 확장하여 사용
 */
USTRUCT(meta = (DisplayName = "AO AI Base Context Evaluator"))
struct AO_API FAO_STEval_AIBaseContext : public FStateTreeEvaluatorCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAO_STEval_AIBaseContext_InstanceData;

	FAO_STEval_AIBaseContext() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual void TreeStart(FStateTreeExecutionContext& Context) const override;
	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

protected:
	AAO_AICharacterBase* GetAICharacter(FStateTreeExecutionContext& Context) const;
	AAO_AIControllerBase* GetAIController(FStateTreeExecutionContext& Context) const;
	void UpdateContextData(FStateTreeExecutionContext& Context, FAO_STEval_AIBaseContext_InstanceData& InstanceData) const;
};

