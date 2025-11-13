// AO_BTT_WerewolfHowl.h

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "AO_BTT_WerewolfHowl.generated.h"

class UBlackboardComponent;
class AAO_Enemy_Werewolf;
class AAO_AIC_Werewolf;

UCLASS()
class AO_API UAO_BTT_WerewolfHowl : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UAO_BTT_WerewolfHowl();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp,
											uint8* NodeMemory) override;

public:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;
	
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector PackTargetLocationKey;
	
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector IsAlertedKey;
	
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector IsHowlSourceKey;
	
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector IsPackNotifiedKey;
};
