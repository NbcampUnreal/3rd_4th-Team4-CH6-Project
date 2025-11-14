// AO_BTT_SetPackPositionReached.cpp

#include "AI/BT/BTTasks/AO_BTT_SetPackPositionReached.h"

#include "BehaviorTree/BlackboardComponent.h"

UAO_BTT_SetPackPositionReached::UAO_BTT_SetPackPositionReached()
{
	NodeName = TEXT("Set Pack Position Reached");

	IsInPackPositionKey.AddBoolFilter(
		this,
		GET_MEMBER_NAME_CHECKED(UAO_BTT_SetPackPositionReached, IsInPackPositionKey)
	);
}

EBTNodeResult::Type UAO_BTT_SetPackPositionReached::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory
)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if(!BlackboardComp || !IsInPackPositionKey.SelectedKeyName.IsValid())
	{
		return EBTNodeResult::Failed;
	}

	BlackboardComp->SetValueAsBool(IsInPackPositionKey.SelectedKeyName, true);

	return EBTNodeResult::Succeeded;
}
