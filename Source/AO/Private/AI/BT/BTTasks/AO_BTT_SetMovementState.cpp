// KSJ : AO_BTT_SetMovementState.cpp

#include "AI/BT/BTTasks/AO_BTT_SetMovementState.h"
#include "AI/Character/AO_Enemy_Crab.h"
#include "AIController.h"

UAO_BTT_SetMovementState::UAO_BTT_SetMovementState()
{
	NodeName = TEXT("Set Movement State");
}

EBTNodeResult::Type UAO_BTT_SetMovementState::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return EBTNodeResult::Failed;

	AAO_Enemy_Crab* Crab = Cast<AAO_Enemy_Crab>(AICon->GetPawn());
	if (!Crab) return EBTNodeResult::Failed;

	Crab->SetFleeMode(bIsFleeing);

	return EBTNodeResult::Succeeded;
}
