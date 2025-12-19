// AO_STEval_AggressiveCtx.h

#pragma once

#include "CoreMinimal.h"
#include "StateTreeEvaluatorBase.h"
#include "AO_STEval_AggressiveCtx.generated.h"

class AAO_AggressiveAIBase;
class AAO_AggressiveAICtrl;
class AAO_PlayerCharacter;

/**
 * 선공형 AI 상태 평가 Evaluator 인스턴스 데이터
 */
USTRUCT()
struct FAO_STEval_AggressiveCtx_InstanceData
{
	GENERATED_BODY()

	// 시야 내 플레이어 존재 여부
	UPROPERTY(EditAnywhere, Category = "Output")
	bool bHasPlayerInSight = false;

	// 현재 추격 중인지
	UPROPERTY(EditAnywhere, Category = "Output")
	bool bIsChasing = false;

	// 현재 수색 중인지
	UPROPERTY(EditAnywhere, Category = "Output")
	bool bIsSearching = false;

	// 대상이 공격 범위 내에 있는지
	UPROPERTY(EditAnywhere, Category = "Output")
	bool bTargetInAttackRange = false;

	// 기절 상태 여부
	UPROPERTY(EditAnywhere, Category = "Output")
	bool bIsStunned = false;

	// 현재 추격 대상
	UPROPERTY(EditAnywhere, Category = "Output")
	TWeakObjectPtr<AAO_PlayerCharacter> CurrentTarget;

	// 마지막으로 플레이어를 본 위치
	UPROPERTY(EditAnywhere, Category = "Output")
	FVector LastKnownTargetLocation = FVector::ZeroVector;

	// 가장 가까운 플레이어까지의 거리
	UPROPERTY(EditAnywhere, Category = "Output")
	float NearestPlayerDistance = MAX_FLT;
};

/**
 * 선공형 AI 상태 평가 Evaluator
 * - 현재 상태를 평가하여 StateTree에 데이터 제공
 * - 추격/수색/공격 상태 관리
 */
USTRUCT(meta = (DisplayName = "AO Aggressive AI Context Evaluator"))
struct AO_API FAO_STEval_AggressiveCtx : public FStateTreeEvaluatorCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAO_STEval_AggressiveCtx_InstanceData;

	FAO_STEval_AggressiveCtx() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual void TreeStart(FStateTreeExecutionContext& Context) const override;
	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

protected:
	AAO_AggressiveAIBase* GetAggressiveAI(FStateTreeExecutionContext& Context) const;
	AAO_AggressiveAICtrl* GetAggressiveController(FStateTreeExecutionContext& Context) const;
	void UpdateContextData(FStateTreeExecutionContext& Context, FAO_STEval_AggressiveCtx_InstanceData& InstanceData) const;
};

