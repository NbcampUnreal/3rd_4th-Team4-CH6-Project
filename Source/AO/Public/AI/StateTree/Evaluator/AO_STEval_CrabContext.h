// AO_STEval_CrabContext.h

#pragma once

#include "CoreMinimal.h"
#include "StateTreeEvaluatorBase.h"
#include "AO_STEval_CrabContext.generated.h"

class AAO_Crab;
class AAO_CrabController;
class AAO_MasterItem;

/**
 * Crab 상태 평가 Evaluator 인스턴스 데이터
 */
USTRUCT()
struct FAO_STEval_CrabContext_InstanceData
{
	GENERATED_BODY()

	// 현재 아이템 소지 여부
	UPROPERTY(EditAnywhere, Category = "Output")
	bool bIsCarryingItem = false;

	// 시야 내 플레이어 존재 여부
	UPROPERTY(EditAnywhere, Category = "Output")
	bool bHasPlayerInSight = false;

	// 위협 감지 여부 (시야 또는 소리)
	UPROPERTY(EditAnywhere, Category = "Output")
	bool bThreatDetected = false;

	// 기절 상태 여부
	UPROPERTY(EditAnywhere, Category = "Output")
	bool bIsStunned = false;

	// 가장 가까운 아이템까지의 거리
	UPROPERTY(EditAnywhere, Category = "Output")
	float NearestItemDistance = MAX_FLT;

	// 가장 가까운 위협까지의 거리
	UPROPERTY(EditAnywhere, Category = "Output")
	float NearestThreatDistance = MAX_FLT;

	// 가장 가까운 위협 위치
	UPROPERTY(EditAnywhere, Category = "Output")
	FVector NearestThreatLocation = FVector::ZeroVector;
};

/**
 * Crab 상태 평가 Evaluator
 * - 현재 상태를 평가하여 StateTree에 데이터 제공
 */
USTRUCT(meta = (DisplayName = "AO Crab Context Evaluator"))
struct AO_API FAO_STEval_CrabContext : public FStateTreeEvaluatorCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAO_STEval_CrabContext_InstanceData;

	FAO_STEval_CrabContext() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual void TreeStart(FStateTreeExecutionContext& Context) const override;
	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

	// 아이템 탐색 반경
	UPROPERTY(EditAnywhere, Category = "Settings")
	float ItemSearchRadius = 1500.f;

	// 위협 감지 반경 (소리 등)
	UPROPERTY(EditAnywhere, Category = "Settings")
	float ThreatDetectionRadius = 1000.f;

protected:
	AAO_Crab* GetCrab(FStateTreeExecutionContext& Context) const;
	AAO_CrabController* GetCrabController(FStateTreeExecutionContext& Context) const;
	void UpdateContextData(FStateTreeExecutionContext& Context, FAO_STEval_CrabContext_InstanceData& InstanceData) const;
};
