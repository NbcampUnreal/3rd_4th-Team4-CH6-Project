// KSJ : AO_BTT_DropItem.cpp

#include "AI/BT/BTTasks/AO_BTT_DropItem.h"
#include "AI/Character/AO_Enemy_Crab.h"
#include "AIController.h"

UAO_BTT_DropItem::UAO_BTT_DropItem()
{
	NodeName = TEXT("Drop Item");
}

EBTNodeResult::Type UAO_BTT_DropItem::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return EBTNodeResult::Failed;

	AAO_Enemy_Crab* Crab = Cast<AAO_Enemy_Crab>(AICon->GetPawn());
	if (!Crab) return EBTNodeResult::Failed;

	// 떨어뜨리기 실행
	Crab->DropItem();

	return EBTNodeResult::Succeeded;
}
