// KSJ : AO_BTT_TrollAttack.cpp

#include "AI/BT/BTTasks/AO_BTT_TrollAttack.h"
#include "AI/Character/AO_Enemy_Troll.h"
#include "AIController.h"

UAO_BTT_TrollAttack::UAO_BTT_TrollAttack()
{
	NodeName = TEXT("Troll Attack");
}

EBTNodeResult::Type UAO_BTT_TrollAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return EBTNodeResult::Failed;

	AAO_Enemy_Troll* Troll = Cast<AAO_Enemy_Troll>(AICon->GetPawn());
	if (!Troll) return EBTNodeResult::Failed;

	Troll->PerformAttack();

	return EBTNodeResult::Succeeded;
}