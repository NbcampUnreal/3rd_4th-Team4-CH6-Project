//AO_BTS_SyncHasItem.h

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "AO_BTS_SyncHasItem.generated.h"

class AAO_Enemy_Crab;
class AAO_TestPickupItem;

UCLASS()
class AO_API UAO_BTS_SyncHasItem : public UBTService
{
	GENERATED_BODY()

public:
	UAO_BTS_SyncHasItem();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

public:
	/** bHasItem(bool) 키 */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector HasItemKey;

	/** TargetItem(Object: AO_PickupItem) — 들고 있는 동안이면 비움 */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector TargetItemKey;
};