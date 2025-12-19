// AO_STTask_AIRoam.h

#pragma once

#include "CoreMinimal.h"
#include "Tasks/StateTreeAITask.h"
#include "AO_STTask_AIRoam.generated.h"

class AAIController;

/**
 * AI 배회 Task 인스턴스 데이터
 */
USTRUCT()
struct FAO_STTask_AIRoam_InstanceData
{
	GENERATED_BODY()

	// 배회 반경
	UPROPERTY(EditAnywhere, Category = "Roam")
	float RoamRadius = 1000.f;

	// 도착 허용 반경
	UPROPERTY(EditAnywhere, Category = "Roam")
	float AcceptanceRadius = 100.f;

	// 목적지 도착 후 대기 시간
	UPROPERTY(EditAnywhere, Category = "Roam")
	float WaitTimeAtDestination = 2.f;

	// 현재 상태
	UPROPERTY()
	bool bIsMoving = false;

	UPROPERTY()
	bool bIsWaiting = false;

	UPROPERTY()
	float WaitTimer = 0.f;
};

/**
 * AI 배회 Task
 * - 랜덤한 위치로 이동하며 배회
 * - NavMesh 내에서 유효한 위치 탐색
 */
USTRUCT(meta = (DisplayName = "AO AI Roam", Category = "AI|AO"))
struct AO_API FAO_STTask_AIRoam : public FStateTreeAIActionTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAO_STTask_AIRoam_InstanceData;

	FAO_STTask_AIRoam() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

protected:
	AAIController* GetAIController(FStateTreeExecutionContext& Context) const;
	FVector FindRandomRoamLocation(FStateTreeExecutionContext& Context, float Radius) const;
	bool StartMovingToLocation(AAIController* AIController, const FVector& Location, float AcceptRadius) const;
};
