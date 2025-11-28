// KSJ : AO_BTT_ClearKey.h

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "AO_BTT_ClearKey.generated.h"

/**
 * 지정된 블랙보드 키를 초기화(Clear)하는 Task
 */
UCLASS()
class AO_API UAO_BTT_ClearKey : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UAO_BTT_ClearKey();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector BlackboardKey;
};
