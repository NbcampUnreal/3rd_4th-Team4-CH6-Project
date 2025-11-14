// AO_BTT_UseWalkSpeed.h

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "AO_BTT_UseWalkSpeed.generated.h"

UCLASS()
class AO_API UAO_BTT_UseWalkSpeed : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UAO_BTT_UseWalkSpeed();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};