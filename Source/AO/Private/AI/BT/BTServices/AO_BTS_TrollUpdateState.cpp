// KSJ : AO_BTS_TrollUpdateState.cpp

#include "AI/BT/BTServices/AO_BTS_TrollUpdateState.h"
#include "AI/Character/AO_Enemy_Troll.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UAO_BTS_TrollUpdateState::UAO_BTS_TrollUpdateState()
{
	NodeName = TEXT("Update Troll Speed");
	Interval = 0.5f;
	RandomDeviation = 0.1f;
}

void UAO_BTS_TrollUpdateState::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return;

	AAO_Enemy_Troll* Troll = Cast<AAO_Enemy_Troll>(AICon->GetPawn());
	if (!Troll) return;

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard) return;

	UObject* TargetObject = Blackboard->GetValueAsObject(TargetPlayerKey.SelectedKeyName);
	bool bIsChasing = (TargetObject != nullptr);

	Troll->SetMovementMode(bIsChasing);
}