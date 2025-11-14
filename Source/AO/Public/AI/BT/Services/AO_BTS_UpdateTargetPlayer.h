// AO_BTS_UpdateTargetPlayer.h

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "AO_BTS_UpdateTargetPlayer.generated.h"

UCLASS()
class AO_API UAO_BTS_UpdateTargetPlayer : public UBTService
{
	GENERATED_BODY()

public:
	UAO_BTS_UpdateTargetPlayer();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector PlayerActorKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector bPlayerVisibleKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector LastKnownLocationKey;
};
