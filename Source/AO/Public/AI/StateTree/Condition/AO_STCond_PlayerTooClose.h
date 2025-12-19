// AO_STCond_PlayerTooClose.h

#pragma once

#include "CoreMinimal.h"
#include "Conditions/StateTreeAIConditionBase.h"
#include "AO_STCond_PlayerTooClose.generated.h"

class AAO_AggressiveAICtrl;

/**
 * 플레이어가 너무 가까운지 확인하는 Condition 인스턴스 데이터
 */
USTRUCT()
struct FAO_STCond_PlayerTooClose_InstanceData
{
	GENERATED_BODY()

	// 반전 여부
	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bInvert = false;

	// 최소 안전 거리 (이 거리보다 가까우면 "너무 가까움")
	UPROPERTY(EditAnywhere, Category = "Distance")
	float MinSafeDistance = 500.0f;
};

/**
 * 플레이어가 너무 가까운지 확인하는 Condition
 * - Stalker가 플레이어에게 접근할 때 거리를 벌리기 위해 사용
 */
USTRUCT(meta = (DisplayName = "AO Player Too Close", Category = "AI|AO"))
struct AO_API FAO_STCond_PlayerTooClose : public FStateTreeAIConditionBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAO_STCond_PlayerTooClose_InstanceData;

	FAO_STCond_PlayerTooClose() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

protected:
	AAO_AggressiveAICtrl* GetAggressiveController(FStateTreeExecutionContext& Context) const;
};

