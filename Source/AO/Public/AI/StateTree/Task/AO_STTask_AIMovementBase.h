//KSJ : AO_STTask_AIMovementBase

#pragma once

#include "CoreMinimal.h"
#include "Tasks/StateTreeAITask.h"
#include "AO_STTask_AIMovementBase.generated.h"

class AAIController;
class UPathFollowingComponent;

/**
 * AI 이동 베이스 Task 인스턴스 데이터
 */
USTRUCT()
struct FAO_STTask_AIMovementBase_InstanceData
{
	GENERATED_BODY()

	// 도착 허용 반경
	UPROPERTY(EditAnywhere, Category = "Movement")
	float AcceptanceRadius = 100.f;

	// 이동 중인지 여부
	UPROPERTY()
	bool bIsMoving = false;
};

/**
 * AI 이동 베이스 Task
 * - 공통 이동 로직 제공
 * - 자식 Task에서 상속하여 사용
 */
USTRUCT()
struct AO_API FAO_STTask_AIMovementBase : public FStateTreeAIActionTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAO_STTask_AIMovementBase_InstanceData;

	FAO_STTask_AIMovementBase() = default;

protected:
	// AIController 가져오기 (공통 헬퍼)
	AAIController* GetAIController(FStateTreeExecutionContext& Context) const;

	// 위치로 이동 시작
	bool MoveToLocation(AAIController* Controller, const FVector& Location, float AcceptRadius) const;

	// 이동 완료 확인
	EStateTreeRunStatus CheckMovementStatus(AAIController* Controller) const;

	// 이동 중지
	void StopMovement(AAIController* Controller) const;
};

