// AO_STEval_TrollContext.h

#pragma once

#include "CoreMinimal.h"
#include "StateTreeEvaluatorBase.h"
#include "AO_STEval_TrollContext.generated.h"

class AAO_Troll;
class AAO_TrollController;
class AAO_TrollWeapon;

/**
 * Troll 상태 평가 Evaluator 인스턴스 데이터
 */
USTRUCT()
struct FAO_STEval_TrollContext_InstanceData
{
	GENERATED_BODY()

	// 무기 소지 여부
	UPROPERTY(EditAnywhere, Category = "Output")
	bool bHasWeapon = false;

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

	// 공격 중인지
	UPROPERTY(EditAnywhere, Category = "Output")
	bool bIsAttacking = false;

	// 무기 줍기 중인지
	UPROPERTY(EditAnywhere, Category = "Output")
	bool bIsPickingUpWeapon = false;

	// 기절 상태 여부
	UPROPERTY(EditAnywhere, Category = "Output")
	bool bIsStunned = false;

	// 시야 내 무기 존재 여부
	UPROPERTY(EditAnywhere, Category = "Output")
	bool bHasWeaponInSight = false;

	// 무기 줍기 우선 여부
	UPROPERTY(EditAnywhere, Category = "Output")
	bool bShouldPrioritizeWeapon = false;

	// 가장 가까운 무기 위치
	UPROPERTY(EditAnywhere, Category = "Output")
	FVector NearestWeaponLocation = FVector::ZeroVector;
};

/**
 * Troll 상태 평가 Evaluator
 * - Troll 전용 상태를 평가하여 StateTree에 데이터 제공
 */
USTRUCT(meta = (DisplayName = "AO Troll Context Evaluator"))
struct AO_API FAO_STEval_TrollContext : public FStateTreeEvaluatorCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAO_STEval_TrollContext_InstanceData;

	FAO_STEval_TrollContext() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual void TreeStart(FStateTreeExecutionContext& Context) const override;
	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

protected:
	AAO_Troll* GetTroll(FStateTreeExecutionContext& Context) const;
	AAO_TrollController* GetTrollController(FStateTreeExecutionContext& Context) const;
	void UpdateContextData(FStateTreeExecutionContext& Context, FAO_STEval_TrollContext_InstanceData& InstanceData) const;
};

