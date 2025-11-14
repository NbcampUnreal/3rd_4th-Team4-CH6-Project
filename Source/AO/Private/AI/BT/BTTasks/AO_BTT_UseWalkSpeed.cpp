// AO_BTT_UseWalkSpeed.cpp

#include "AO/Public/AI/BT/BTTasks/AO_BTT_UseWalkSpeed.h"
#include "AO/Public/AI/Character/AO_Enemy_Crab.h"
#include "AIController.h"

UAO_BTT_UseWalkSpeed::UAO_BTT_UseWalkSpeed()
{
	NodeName = TEXT("Use Walk Speed");
}

EBTNodeResult::Type UAO_BTT_UseWalkSpeed::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AAIController* AICon = OwnerComp.GetAIOwner();
	AAO_Enemy_Crab* Crab = AICon ? Cast<AAO_Enemy_Crab>(AICon->GetPawn()) : nullptr;

	if (Crab == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	Crab->UseWalkSpeed();
	return EBTNodeResult::Succeeded;
}
