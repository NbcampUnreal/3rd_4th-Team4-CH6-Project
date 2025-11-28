// KSJ : AO_BTT_GrabItem.h

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "AO_BTT_GrabItem.generated.h"

/**
 * Crab 전용 Task: 블랙보드에 지정된 아이템을 줍는다.
 */
UCLASS()
class AO_API UAO_BTT_GrabItem : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UAO_BTT_GrabItem();
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override;
};
