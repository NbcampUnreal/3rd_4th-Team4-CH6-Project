// AO_STCond_CanAmbush.h

#pragma once

#include "CoreMinimal.h"
#include "Conditions/StateTreeAIConditionBase.h"
#include "AO_STCond_CanAmbush.generated.h"

class AAO_AggressiveAICtrl;

/**
 * Stalker가 기습 공격 가능한 위치에 있는지 확인하는 Condition 인스턴스 데이터
 * (플레이어의 등 뒤나 옆)
 */
USTRUCT()
struct FAO_STCond_CanAmbush_InstanceData
{
	GENERATED_BODY()

	// 반전 여부
	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bInvert = false;

	// 등 뒤 각도 (Cosine 값, -1.0 ~ 0.0)
	// -0.3 = 약 110도 (등 뒤)
	UPROPERTY(EditAnywhere, Category = "Ambush")
	float BackAngleCosine = -0.3f;

	// 옆 각도 (Cosine 값, 0.0 ~ 1.0)
	// 0.0 = 90도 (옆)
	UPROPERTY(EditAnywhere, Category = "Ambush")
	float SideAngleCosine = 0.0f;
};

/**
 * Stalker가 기습 공격 가능한 위치에 있는지 확인하는 Condition
 * - 플레이어의 등 뒤나 옆에 있는지 체크
 */
USTRUCT(meta = (DisplayName = "AO Can Ambush", Category = "AI|AO"))
struct AO_API FAO_STCond_CanAmbush : public FStateTreeAIConditionBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAO_STCond_CanAmbush_InstanceData;

	FAO_STCond_CanAmbush() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

protected:
	AAO_AggressiveAICtrl* GetAggressiveController(FStateTreeExecutionContext& Context) const;
};

