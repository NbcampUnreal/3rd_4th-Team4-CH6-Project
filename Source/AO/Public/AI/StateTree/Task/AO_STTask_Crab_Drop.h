// AO_STTask_Crab_Drop.h

#pragma once

#include "CoreMinimal.h"
#include "Tasks/StateTreeAITask.h"
#include "AO_STTask_Crab_Drop.generated.h"

class AAO_Crab;

/**
 * Crab 아이템 드롭 Task 인스턴스 데이터
 */
USTRUCT()
struct FAO_STTask_Crab_Drop_InstanceData
{
	GENERATED_BODY()

	// 드롭 위치 도착 허용 반경
	UPROPERTY(EditAnywhere, Category = "Drop")
	float AcceptanceRadius = 100.f;

	// 목표 드롭 위치
	UPROPERTY()
	FVector DropLocation = FVector::ZeroVector;

	// 이동 중인지
	UPROPERTY()
	bool bIsMovingToDropLocation = false;
};

/**
 * Crab 아이템 드롭 Task
 * - 플레이어들로부터 먼 위치로 이동
 * - 아이템 드롭
 */
USTRUCT(meta = (DisplayName = "AO Crab Drop Item", Category = "AI|AO|Crab"))
struct AO_API FAO_STTask_Crab_Drop : public FStateTreeAIActionTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAO_STTask_Crab_Drop_InstanceData;

	FAO_STTask_Crab_Drop() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

protected:
	AAO_Crab* GetCrab(FStateTreeExecutionContext& Context) const;
};
