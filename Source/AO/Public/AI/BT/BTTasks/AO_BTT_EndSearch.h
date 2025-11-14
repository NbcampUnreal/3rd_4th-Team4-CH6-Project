// AO_BTT_EndSearch.h

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "AO_BTT_EndSearch.generated.h"

UCLASS()
class AO_API UAO_BTT_EndSearch : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UAO_BTT_EndSearch();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp,
											uint8* NodeMemory) override;

public:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector IsSearchingKey;
	
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector LastSeenLocationKey;
	
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector MoveToLocationKey;
	
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;
};
