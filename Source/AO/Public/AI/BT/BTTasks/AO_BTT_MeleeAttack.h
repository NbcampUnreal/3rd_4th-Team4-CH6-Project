// AO_BTT_MeleeAttack.h

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "AO_BTT_MeleeAttack.generated.h"

class UBlackboardComponent;
class AAO_Enemy_HumanoidBase;

UCLASS()
class AO_API UAO_BTT_MeleeAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UAO_BTT_MeleeAttack();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp,
											uint8* NodeMemory) override;

public:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;
};