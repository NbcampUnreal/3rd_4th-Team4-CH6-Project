// AO_BTT_GrabItem.h

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "AO_BTT_GrabItem.generated.h"

class AAO_TestPickupItem;

UCLASS()
class AO_API UAO_BTT_GrabItem : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UAO_BTT_GrabItem();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory) override;

public:
	UPROPERTY(EditAnywhere, Category = "BlackBoard")
	struct FBlackboardKeySelector TargetItemKey;

	UPROPERTY(EditAnywhere, Category = "BlackBoard")
	struct FBlackboardKeySelector HasItemKey;
};