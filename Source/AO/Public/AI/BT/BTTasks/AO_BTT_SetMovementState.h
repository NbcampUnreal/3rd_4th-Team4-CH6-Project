// KSJ : AO_BTT_SetMovementState.h

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "AO_BTT_SetMovementState.generated.h"

/**
 * Crab의 이동 상태(도주/일반)를 설정하는 태스크
 */
UCLASS()
class AO_API UAO_BTT_SetMovementState : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UAO_BTT_SetMovementState();
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	// true: 도망 모드 (빠름), false: 일반 모드 (보통)
	UPROPERTY(EditAnywhere, Category = "AI")
	bool bIsFleeing = false;
};
