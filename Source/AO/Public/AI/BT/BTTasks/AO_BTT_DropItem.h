// AO_BTT_DropItem.h

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "GameplayTagContainer.h"
#include "AO_BTT_DropItem.generated.h"

class AAO_TestPickupItem;

UCLASS()
class AO_API UAO_BTT_DropItem : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UAO_BTT_DropItem();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

public:
	/** bHasItem(bool) */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector HasItemKey;

	/** LastDroppedItem(Object: AO_PickupItem) */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector LastDroppedItemKey;

	/** 드랍 직후 부여할 태그(Crab.RecentlyDropped) */
	UPROPERTY(EditAnywhere, Category = "Tags")
	FGameplayTag RecentlyDroppedTag;

	/** 태그 유지 시간(초). 경과 후 자동 제거 */
	UPROPERTY(EditAnywhere, Category = "Tags")
	float RecentlyDroppedTagDuration;

private:
	void ScheduleTagRemoval(AAO_TestPickupItem* Item, const FGameplayTag Tag, float Duration) const;
};