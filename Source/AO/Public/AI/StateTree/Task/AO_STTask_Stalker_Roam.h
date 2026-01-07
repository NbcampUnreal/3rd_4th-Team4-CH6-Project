//KSJ : AO_STTask_Stalker_Roam

#pragma once

#include "CoreMinimal.h"
#include "Tasks/StateTreeAITask.h"
#include "AO_STTask_Stalker_Roam.generated.h"

class AAIController;
class UAO_CeilingMoveComponent;

/**
 * Stalker 전용 배회 Task 인스턴스 데이터
 */
USTRUCT()
struct FAO_STTask_Stalker_Roam_InstanceData
{
	GENERATED_BODY()

	// 배회 반경
	UPROPERTY(EditAnywhere, Category = "Roam")
	float RoamRadius = 1500.f;

	// 도착 허용 반경
	UPROPERTY(EditAnywhere, Category = "Roam")
	float AcceptanceRadius = 100.f;

	// 목적지 도착 후 대기 시간
	UPROPERTY(EditAnywhere, Category = "Roam")
	float WaitTimeAtDestination = 2.f;

	// 천장 이동 전환 확률 (0.0 ~ 1.0)
	// 대기 상태가 끝날 때마다 체크합니다.
	UPROPERTY(EditAnywhere, Category = "Ceiling", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CeilingMoveChance = 0.5f;

	// 현재 상태
	UPROPERTY()
	bool bIsMoving = false;

	UPROPERTY()
	bool bIsWaiting = false;

	UPROPERTY()
	float WaitTimer = 0.f;

	UPROPERTY()
	TObjectPtr<class AAO_Stalker> Stalker;

	UPROPERTY()
	TObjectPtr<UAO_CeilingMoveComponent> CeilingComp;

	// 전환 몽타주 재생 중이면 이동을 잠시 멈추고, 종료 후 MoveTo를 다시 건다.
	UPROPERTY()
	bool bWaitingForTransition = false;

	// 전환이 끝난 뒤 이동할 목적지
	UPROPERTY()
	FVector PendingRoamLocation = FVector::ZeroVector;
};

/**
 * Stalker 전용 Roam Task
 * - 배회 중 천장이 감지되면 확률적으로 천장 이동 모드로 전환
 * - 천장이 끊기면 자동으로 바닥으로 전환
 */
USTRUCT(meta = (DisplayName = "AO Stalker Roam", Category = "AI|AO|Stalker"))
struct AO_API FAO_STTask_Stalker_Roam : public FStateTreeAIActionTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAO_STTask_Stalker_Roam_InstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

protected:
	AAIController* GetAIController(FStateTreeExecutionContext& Context) const;
	FVector FindRandomRoamLocation(FStateTreeExecutionContext& Context, float Radius) const;
	bool StartMovingToLocation(AAIController* AIController, const FVector& Location, float AcceptRadius) const;
};
