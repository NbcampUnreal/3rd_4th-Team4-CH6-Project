// KSJ : AO_BTT_TrollConfirmTarget.cpp

#include "AI/BT/BTTasks/AO_BTT_TrollConfirmTarget.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "AO_Log.h"

UAO_BTT_TrollConfirmTarget::UAO_BTT_TrollConfirmTarget()
{
	NodeName = TEXT("Confirm New Target");
}

EBTNodeResult::Type UAO_BTT_TrollConfirmTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard) return EBTNodeResult::Failed;

	UObject* PotentialObj = Blackboard->GetValueAsObject(TEXT("PotentialTarget"));
	AActor* NewTarget = Cast<AActor>(PotentialObj);

	if (NewTarget)
	{
		Blackboard->SetValueAsObject(TEXT("TargetPlayer"), NewTarget);
		AO_LOG(LogKSJ, Log, TEXT("Troll switched target to %s"), *NewTarget->GetName());
	}

	Blackboard->ClearValue(TEXT("bIsHesitating"));
	Blackboard->ClearValue(TEXT("PotentialTarget"));

	return EBTNodeResult::Succeeded;
}