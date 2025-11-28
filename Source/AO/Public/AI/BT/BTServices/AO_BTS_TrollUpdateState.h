// KSJ : AO_BTS_TrollUpdateState.h

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "AO_BTS_TrollUpdateState.generated.h"

/**
 * Troll의 상태(추격/배회)를 확인하고 이동 속도를 갱신하는 서비스
 */
UCLASS()
class AO_API UAO_BTS_TrollUpdateState : public UBTService
{
	GENERATED_BODY()

public:
	UAO_BTS_TrollUpdateState();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector TargetPlayerKey;
};