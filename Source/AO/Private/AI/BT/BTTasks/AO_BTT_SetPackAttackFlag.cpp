// AO_BTT_SetPackAttackFlag.cpp

#include "AI/BT/BTTasks/AO_BTT_SetPackAttackFlag.h"

#include "BehaviorTree/BlackboardComponent.h"

UAO_BTT_SetPackAttackFlag::UAO_BTT_SetPackAttackFlag()
{
	NodeName = TEXT("Set Pack Attack Flag");

	ShouldStartPackAttackKey.AddBoolFilter(
		this,
		GET_MEMBER_NAME_CHECKED(UAO_BTT_SetPackAttackFlag, ShouldStartPackAttackKey)
	);
}

EBTNodeResult::Type UAO_BTT_SetPackAttackFlag::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory
)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if(!BlackboardComp || !ShouldStartPackAttackKey.SelectedKeyName.IsValid())
	{
		return EBTNodeResult::Failed;
	}

	BlackboardComp->SetValueAsBool(ShouldStartPackAttackKey.SelectedKeyName, true);

	return EBTNodeResult::Succeeded;
}
