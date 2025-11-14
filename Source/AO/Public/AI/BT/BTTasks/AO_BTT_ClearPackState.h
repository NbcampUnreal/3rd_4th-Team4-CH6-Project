// AO_BTT_ClearPackState.h

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "AO_BTT_ClearPackState.generated.h"

UCLASS()
class AO_API UAO_BTT_ClearPackState : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UAO_BTT_ClearPackState();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp,
											uint8* NodeMemory) override;

public:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector IsPackNotifiedKey;
	
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector IsInPackPositionKey;
	
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector ShouldStartPackAttackKey;
	
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector IsHowlSourceKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector HasPackMates;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector HasUsedPackAttackKey;
};
