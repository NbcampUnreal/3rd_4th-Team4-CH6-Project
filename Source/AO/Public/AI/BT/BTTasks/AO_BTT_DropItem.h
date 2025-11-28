// KSJ : AO_BTT_DropItem.h

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "AO_BTT_DropItem.generated.h"

/**
 * Crab 전용 Task: 현재 들고 있는 아이템을 떨어뜨린다.
 */
UCLASS()
class AO_API UAO_BTT_DropItem : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UAO_BTT_DropItem();
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
