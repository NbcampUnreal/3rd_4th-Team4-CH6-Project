// AO_BTS_UpdateTarget.h

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "AO_BTS_UpdateTarget.generated.h"

UCLASS()
class AO_API UAO_BTS_UpdateTarget : public UBTService
{
	GENERATED_BODY()

public:
	UAO_BTS_UpdateTarget();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp,
						  uint8* NodeMemory,
						  float DeltaSeconds) override;

public:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;
	
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector HasLineOfSightKey;
	
	UPROPERTY(EditAnywhere, Category = "AI")
	float MaxTargetDistance = 5000.0f;
};
