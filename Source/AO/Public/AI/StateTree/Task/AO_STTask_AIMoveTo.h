// AO_STTask_AIMoveTo.h

#pragma once

#include "CoreMinimal.h"
#include "Tasks/StateTreeAITask.h"
#include "AO_STTask_AIMoveTo.generated.h"

class AAIController;

/**
 * AI 이동 Task 인스턴스 데이터
 */
USTRUCT()
struct FAO_STTask_AIMoveTo_InstanceData
{
	GENERATED_BODY()

	// 목표 위치
	UPROPERTY(EditAnywhere, Category = "Movement")
	FVector TargetLocation = FVector::ZeroVector;

	// 도착 허용 반경
	UPROPERTY(EditAnywhere, Category = "Movement")
	float AcceptanceRadius = 100.f;

	// 이동 중인지 여부
	UPROPERTY()
	bool bIsMoving = false;
};

/**
 * AI 이동 Task
 * - 지정된 위치로 AI를 이동시킴
 * - NavMesh 기반 경로 탐색
 */
USTRUCT(meta = (DisplayName = "AO AI Move To", Category = "AI|AO"))
struct AO_API FAO_STTask_AIMoveTo : public FStateTreeAIActionTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAO_STTask_AIMoveTo_InstanceData;

	FAO_STTask_AIMoveTo() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

protected:
	AAIController* GetAIController(FStateTreeExecutionContext& Context) const;
};
