// AO_BTT_SetPackPositionReached.h

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "AO_BTT_SetPackPositionReached.generated.h"

UCLASS()
class AO_API UAO_BTT_SetPackPositionReached : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UAO_BTT_SetPackPositionReached();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp,
											uint8* NodeMemory) override;

public:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector IsInPackPositionKey;
};
