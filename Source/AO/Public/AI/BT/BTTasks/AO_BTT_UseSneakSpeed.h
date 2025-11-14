// AO_BTT_UseSneakSpeed.h

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "AO_BTT_UseSneakSpeed.generated.h"

UCLASS()
class AO_API UAO_BTT_UseSneakSpeed : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UAO_BTT_UseSneakSpeed();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};