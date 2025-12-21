// AO_STTask_Crab_Pickup.h

#pragma once

#include "CoreMinimal.h"
#include "Tasks/StateTreeAITask.h"
#include "AO_STTask_Crab_Pickup.generated.h"

class AAO_Crab;
class AAO_MasterItem;

/**
 * Crab 아이템 줍기 Task 인스턴스 데이터
 */
USTRUCT()
struct FAO_STTask_Crab_Pickup_InstanceData
{
	GENERATED_BODY()

	// 아이템 탐색 반경
	UPROPERTY(EditAnywhere, Category = "Pickup")
	float SearchRadius = 1500.f;

	// 아이템 줍기 거리
	UPROPERTY(EditAnywhere, Category = "Pickup")
	float PickupDistance = 150.f;

	// 현재 타겟 아이템
	UPROPERTY()
	TWeakObjectPtr<AAO_MasterItem> TargetItem;

	// 이동 중인지
	UPROPERTY()
	bool bIsMovingToItem = false;
};

/**
 * Crab 아이템 줍기 Task
 * - 주변 아이템 탐색
 * - 아이템으로 이동 후 줍기
 */
USTRUCT(meta = (DisplayName = "AO Crab Pickup Item", Category = "AI|AO|Crab"))
struct AO_API FAO_STTask_Crab_Pickup : public FStateTreeAIActionTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAO_STTask_Crab_Pickup_InstanceData;

	FAO_STTask_Crab_Pickup() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

protected:
	AAO_Crab* GetCrab(FStateTreeExecutionContext& Context) const;
	AAO_MasterItem* FindNearbyItem(FStateTreeExecutionContext& Context, float Radius) const;
};
