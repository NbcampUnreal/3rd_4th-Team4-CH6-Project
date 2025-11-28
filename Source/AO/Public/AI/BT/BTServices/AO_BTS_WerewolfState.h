// KSJ : AO_BTS_WerewolfState.h

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "AO_BTS_WerewolfState.generated.h"

/**
 * PackManager로부터 현재 늑대의 상태(Idle/Siege/Rush)를 받아와
 * 블랙보드에 업데이트하는 서비스
 */
UCLASS()
class AO_API UAO_BTS_WerewolfState : public UBTService
{
	GENERATED_BODY()

public:
	UAO_BTS_WerewolfState();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector PackStateKey;
};
