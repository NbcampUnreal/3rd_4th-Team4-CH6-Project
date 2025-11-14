// AO_BTS_WerewolfPackCoordinator.h

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "AO_BTS_WerewolfPackCoordinator.generated.h"

UCLASS()
class AO_API UAO_BTS_WerewolfPackCoordinator : public UBTService
{
	GENERATED_BODY()

public:
	UAO_BTS_WerewolfPackCoordinator();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp,
						  uint8* NodeMemory,
						  float DeltaSeconds) override;

public:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector IsHowlSourceKey;
	
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector IsPackNotifiedKey;
	
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector IsInPackPositionKey;
	
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector ShouldStartPackAttackKey;
	
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector PackTargetLocationKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector HasPackMatesKey;
	
	UPROPERTY(EditAnywhere, Category = "Pack")
	int32 MinPackSize = 2;
	
	UPROPERTY(EditAnywhere, Category = "Pack")
	float PackRadius = 2000.0f;
};
