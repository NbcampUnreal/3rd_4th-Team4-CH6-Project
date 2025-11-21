// AO_BTT_UseSneakSpeed.cpp

#include "AO/Public/AI/BT/BTTasks/AO_BTT_UseSneakSpeed.h"
#include "AO/Public/AI/Character/AO_Enemy_Crab.h"
#include "AIController.h"

UAO_BTT_UseSneakSpeed::UAO_BTT_UseSneakSpeed()
{
	NodeName = TEXT("Use Sneak Speed");
}

EBTNodeResult::Type UAO_BTT_UseSneakSpeed::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AAIController* AICon = OwnerComp.GetAIOwner();
	AAO_Enemy_Crab* Crab = AICon ? Cast<AAO_Enemy_Crab>(AICon->GetPawn()) : nullptr;

	if (Crab == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	Crab->UseSneakSpeed();
	return EBTNodeResult::Succeeded;
}
