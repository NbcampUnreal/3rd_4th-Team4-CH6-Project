// AO_BTT_SetMovementState.cpp

#include "AI/BT/BTTasks/AO_BTT_SetMovementState.h"
#include "AIController.h"
#include "AI/Character/AO_Enemy_HumanoidBase.h"

UAO_BTT_SetMovementState::UAO_BTT_SetMovementState()
{
	NodeName = TEXT("Set Movement State");
}

EBTNodeResult::Type UAO_BTT_SetMovementState::ExecuteTask(UBehaviorTreeComponent& OwnerComp,
														  uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	AAIController* AIController = OwnerComp.GetAIOwner();
	if(!AIController)
	{
		return EBTNodeResult::Failed;
	}

	AAO_Enemy_HumanoidBase* Humanoid =
		Cast<AAO_Enemy_HumanoidBase>(AIController->GetPawn());
	if(!Humanoid)
	{
		return EBTNodeResult::Failed;
	}

	if(Humanoid->GetLocalRole() != ROLE_Authority)
	{
		return EBTNodeResult::Failed;
	}

	Humanoid->SetMovementState(NewMovementState);

	return EBTNodeResult::Succeeded;
}