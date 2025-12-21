// AO_STCond_PlayerNearby.h

#pragma once

#include "CoreMinimal.h"
#include "Conditions/StateTreeAIConditionBase.h"
#include "AO_STCond_PlayerNearby.generated.h"

class AAO_AIControllerBase;

/**
 * 플레이어가 근처에 있는지 확인하는 Condition 인스턴스 데이터
 */
USTRUCT()
struct FAO_STCond_PlayerNearby_InstanceData
{
	GENERATED_BODY()

	// 감지 거리 (시야 외 거리 체크용)
	UPROPERTY(EditAnywhere, Category = "Condition")
	float DetectionRadius = 1000.f;

	// 시야 내 플레이어만 체크할지
	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bOnlySightCheck = false;

	// 반전 여부
	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bInvert = false;
};

/**
 * 플레이어가 근처에 있는지 확인하는 Condition
 * - 시야 내 플레이어 확인
 * - 소리로 감지된 플레이어 위치 확인
 */
USTRUCT(meta = (DisplayName = "AO Player Nearby", Category = "AI|AO"))
struct AO_API FAO_STCond_PlayerNearby : public FStateTreeAIConditionBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAO_STCond_PlayerNearby_InstanceData;

	FAO_STCond_PlayerNearby() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

protected:
	AAO_AIControllerBase* GetAIController(FStateTreeExecutionContext& Context) const;
};
