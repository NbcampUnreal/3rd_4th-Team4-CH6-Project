// KSJ : AO_BTT_SiegeMove.h

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_MoveTo.h"
#include "AO_BTT_SiegeMove.generated.h"

/**
 * 웨어울프 포위 이동 Task
 * 
 * - 일반 MoveTo와 유사하지만, "플레이어와의 거리"를 지속적으로 체크
 * - 이동 중 플레이어가 너무 가까워지면(돌파 시도) 즉시 Fail시켜서 공격으로 유도
 * - PackManager로부터 할당받은 위치로 이동 (지금은 EQS 결과를 블랙보드에서 받아온다고 가정)
 */
UCLASS()
class AO_API UAO_BTT_SiegeMove : public UBTTask_MoveTo
{
	GENERATED_BODY()

public:
	UAO_BTT_SiegeMove();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
	// 돌파 허용 거리 (이 거리 안으로 들어오면 포위 풀고 공격)
	UPROPERTY(EditAnywhere, Category = "Siege")
	float BreachDistance = 300.0f;
};
