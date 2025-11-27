// KSJ : AO_BTT_InsectMoveWithKidnap.h

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_MoveTo.h"
#include "AO_BTT_InsectMoveWithKidnap.generated.h"

/**
 * Insect 납치 중 이동 Task
 * - 뒷걸음질로 목표 위치까지 이동
 * - EQS로 계산된 목표 위치 사용
 */
UCLASS()
class AO_API UAO_BTT_InsectMoveWithKidnap : public UBTTask_MoveTo
{
	GENERATED_BODY()

public:
	UAO_BTT_InsectMoveWithKidnap();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector KidnapTargetLocationKey;
	
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector KidnapStartLocationKey;
	
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector TargetPlayerKey;

private:
	/** 목표 위치 갱신 주기 (초) */
	UPROPERTY(EditAnywhere, Category = "Kidnap")
	float TargetUpdateInterval = 1.0f;
	
	double LastTargetUpdateTime = 0.0;
};
