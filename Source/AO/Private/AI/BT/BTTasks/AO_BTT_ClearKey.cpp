// KSJ : AO_BTT_ClearKey.cpp

#include "AI/BT/BTTasks/AO_BTT_ClearKey.h"
#include "BehaviorTree/BlackboardComponent.h"

UAO_BTT_ClearKey::UAO_BTT_ClearKey()
{
	NodeName = "Clear Key";
}

EBTNodeResult::Type UAO_BTT_ClearKey::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (BB)
	{
		BB->ClearValue(BlackboardKey.SelectedKeyName);
		return EBTNodeResult::Succeeded;
	}
	return EBTNodeResult::Failed;
}
