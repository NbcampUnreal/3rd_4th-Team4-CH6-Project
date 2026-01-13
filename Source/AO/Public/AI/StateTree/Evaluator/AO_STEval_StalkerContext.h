//KSJ : AO_STEval_StalkerContext

#pragma once

#include "CoreMinimal.h"
#include "AI/StateTree/Evaluator/AO_STEval_AggressiveCtx.h"
#include "AO_STEval_StalkerContext.generated.h"

class AAO_StalkerController;

/**
 * Stalker AI 전용 상태 평가 데이터
 */
USTRUCT()
struct FAO_STEval_StalkerCtx_InstanceData : public FAO_STEval_AggressiveCtx_InstanceData
{
	GENERATED_BODY()

	// 계산된 엄폐 위치
	UPROPERTY(EditAnywhere, Category = "Output")
	FVector HideLocation = FVector::ZeroVector;

	// 계산된 도주 위치
	UPROPERTY(EditAnywhere, Category = "Output")
	FVector RetreatLocation = FVector::ZeroVector;

	// 플레이어가 나를 보고 있는지 여부
	UPROPERTY(EditAnywhere, Category = "Output")
	bool bIsPlayerLookingAtMe = false;

	// 나를 보고 있는 플레이어 (가장 가까운 대상)
	UPROPERTY(EditAnywhere, Category = "Output")
	TWeakObjectPtr<AActor> LookingPlayer = nullptr;

	// 도주 중 상태
	UPROPERTY(EditAnywhere, Category = "Output")
	bool bIsRetreating = false;
};

/**
 * Stalker AI Evaluator
 * - Stalker 특화 로직 (엄폐, 도주, 시선 감지) 데이터 제공
 */
USTRUCT(meta = (DisplayName = "AO Stalker Context Evaluator"))
struct AO_API FAO_STEval_StalkerContext : public FAO_STEval_AggressiveCtx
{
	GENERATED_BODY()

	using FInstanceDataType = FAO_STEval_StalkerCtx_InstanceData;

	FAO_STEval_StalkerContext() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual void TreeStart(FStateTreeExecutionContext& Context) const override;
	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

protected:
	// KSJ: DeltaTime 추가 - Hysteresis 로직에 필요
	void UpdateStalkerContextData(FStateTreeExecutionContext& Context, FAO_STEval_StalkerCtx_InstanceData& InstanceData, float DeltaTime) const;
};


