// AO_STTask_Crab_Flee.h

#pragma once

#include "CoreMinimal.h"
#include "Tasks/StateTreeAITask.h"
#include "AO_STTask_Crab_Flee.generated.h"

class AAO_Crab;

/**
 * Crab 도주 Task 인스턴스 데이터
 */
USTRUCT()
struct FAO_STTask_Crab_Flee_InstanceData
{
	GENERATED_BODY()

	// 도주 위치 재계산 간격
	UPROPERTY(EditAnywhere, Category = "Flee")
	float RecalculateInterval = 1.5f;

	// 도착 허용 반경
	UPROPERTY(EditAnywhere, Category = "Flee")
	float AcceptanceRadius = 100.f;

	// 안전 거리 (플레이어와 이 거리 이상 떨어지면 도주 종료)
	UPROPERTY(EditAnywhere, Category = "Flee")
	float SafeDistance = 1500.f;

	// 현재 도주 위치
	UPROPERTY()
	FVector FleeLocation = FVector::ZeroVector;

	// 마지막으로 감지한 위협 위치 (이 위치에서 SafeDistance만큼 멀어져야 도주 종료)
	UPROPERTY()
	FVector LastThreatLocation = FVector::ZeroVector;

	// 재계산 타이머
	UPROPERTY()
	float RecalculateTimer = 0.f;

	// 이동 중인지
	UPROPERTY()
	bool bIsFleeing = false;
};

/**
 * Crab 도주 Task
 * - 플레이어로부터 도망
 * - 피해야 할 위치 고려
 */
USTRUCT(meta = (DisplayName = "AO Crab Flee", Category = "AI|AO|Crab"))
struct AO_API FAO_STTask_Crab_Flee : public FStateTreeAIActionTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAO_STTask_Crab_Flee_InstanceData;

	FAO_STTask_Crab_Flee() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

protected:
	AAO_Crab* GetCrab(FStateTreeExecutionContext& Context) const;
	bool IsPlayerNearby(FStateTreeExecutionContext& Context, float SafeDist) const;
};
