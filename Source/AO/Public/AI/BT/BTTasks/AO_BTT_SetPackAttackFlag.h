// AO_BTT_SetPackAttackFlag.h

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "AO_BTT_SetPackAttackFlag.generated.h"

UCLASS()
class AO_API UAO_BTT_SetPackAttackFlag : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UAO_BTT_SetPackAttackFlag();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp,
											uint8* NodeMemory) override;

public:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector ShouldStartPackAttackKey;
};
