// AO_BTT_WerewolfHowl.h

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "AO_BTT_WerewolfHowl.generated.h"

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
	FBlackboardKeySelector IsHowlSourceKey;
	
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector IsPackNotifiedKey;
	
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector PackTargetLocationKey;
	
	UPROPERTY(EditAnywhere, Category = "AI")
	float HowlNotifyRadius = 3000.0f; // 추측입니다
};
