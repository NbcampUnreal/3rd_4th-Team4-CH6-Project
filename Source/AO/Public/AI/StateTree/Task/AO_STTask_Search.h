// AO_STTask_Search.h

#pragma once

#include "CoreMinimal.h"
#include "Tasks/StateTreeAITask.h"
#include "AO_STTask_Search.generated.h"

class AAO_AggressiveAICtrl;

/**
 * 선공형 AI 수색 Task 인스턴스 데이터
 */
USTRUCT()
struct FAO_STTask_Search_InstanceData
{
	GENERATED_BODY()

	// 수색 반경
	UPROPERTY(EditAnywhere, Category = "Search")
	float SearchRadius = 500.f;

	// 도착 허용 반경
	UPROPERTY(EditAnywhere, Category = "Search")
	float AcceptanceRadius = 100.f;

	// 수색 시간 (5~10초 랜덤)
	UPROPERTY()
	float SearchDuration = 7.f;

	// 남은 수색 시간
	UPROPERTY()
	float RemainingSearchTime = 0.f;

	// 현재 이동 중인지
	UPROPERTY()
	bool bIsMoving = false;

	// 수색 시작 위치
	UPROPERTY()
	FVector SearchCenterLocation = FVector::ZeroVector;
};

/**
 * 선공형 AI 수색 Task
 * - 마지막으로 플레이어를 본 위치 주변 수색
 * - 일정 시간 후 배회로 전환
 */
USTRUCT(meta = (DisplayName = "AO AI Search", Category = "AI|AO"))
struct AO_API FAO_STTask_Search : public FStateTreeAIActionTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAO_STTask_Search_InstanceData;

	FAO_STTask_Search() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

protected:
	AAO_AggressiveAICtrl* GetAggressiveController(FStateTreeExecutionContext& Context) const;
	FVector FindRandomSearchLocation(FStateTreeExecutionContext& Context, const FVector& Center, float Radius) const;
	bool MoveToLocation(AAO_AggressiveAICtrl* Controller, const FVector& Location, float AcceptRadius) const;
};

