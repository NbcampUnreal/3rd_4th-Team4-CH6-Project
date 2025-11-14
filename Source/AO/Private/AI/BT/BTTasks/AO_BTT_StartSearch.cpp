// AO_BTT_StartSearch.cpp

#include "AI/BT/BTTasks/AO_BTT_StartSearch.h"

#include "BehaviorTree/BlackboardComponent.h"

UAO_BTT_StartSearch::UAO_BTT_StartSearch()
{
	NodeName = TEXT("Start Search");

	LastSeenLocationKey.AddVectorFilter(
		this,
		GET_MEMBER_NAME_CHECKED(UAO_BTT_StartSearch, LastSeenLocationKey)
	);

	MoveToLocationKey.AddVectorFilter(
		this,
		GET_MEMBER_NAME_CHECKED(UAO_BTT_StartSearch, MoveToLocationKey)
	);

	IsSearchingKey.AddBoolFilter(
		this,
		GET_MEMBER_NAME_CHECKED(UAO_BTT_StartSearch, IsSearchingKey)
	);
}

EBTNodeResult::Type UAO_BTT_StartSearch::ExecuteTask(UBehaviorTreeComponent& OwnerComp,
													 uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if(!BlackboardComp)
	{
		return EBTNodeResult::Failed;
	}

	if(!LastSeenLocationKey.SelectedKeyName.IsValid() ||
	   !MoveToLocationKey.SelectedKeyName.IsValid() ||
	   !IsSearchingKey.SelectedKeyName.IsValid())
	{
		return EBTNodeResult::Failed;
	}

	const FVector LastSeenLocation =
		BlackboardComp->GetValueAsVector(LastSeenLocationKey.SelectedKeyName);
	
	BlackboardComp->SetValueAsVector(MoveToLocationKey.SelectedKeyName, LastSeenLocation);
	
	BlackboardComp->SetValueAsBool(IsSearchingKey.SelectedKeyName, true);

	return EBTNodeResult::Succeeded;
}
