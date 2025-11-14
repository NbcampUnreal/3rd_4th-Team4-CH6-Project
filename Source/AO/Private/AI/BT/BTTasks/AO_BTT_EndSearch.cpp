// AO_BTT_EndSearch.cpp

#include "AI/BT/BTTasks/AO_BTT_EndSearch.h"

#include "BehaviorTree/BlackboardComponent.h"

UAO_BTT_EndSearch::UAO_BTT_EndSearch()
{
	NodeName = TEXT("End Search");

	IsSearchingKey.AddBoolFilter(
		this,
		GET_MEMBER_NAME_CHECKED(UAO_BTT_EndSearch, IsSearchingKey)
	);

	LastSeenLocationKey.AddVectorFilter(
		this,
		GET_MEMBER_NAME_CHECKED(UAO_BTT_EndSearch, LastSeenLocationKey)
	);

	MoveToLocationKey.AddVectorFilter(
		this,
		GET_MEMBER_NAME_CHECKED(UAO_BTT_EndSearch, MoveToLocationKey)
	);

	TargetActorKey.AddObjectFilter(
		this,
		GET_MEMBER_NAME_CHECKED(UAO_BTT_EndSearch, TargetActorKey),
		AActor::StaticClass()
	);
}

EBTNodeResult::Type UAO_BTT_EndSearch::ExecuteTask(UBehaviorTreeComponent& OwnerComp,
												   uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if(!BlackboardComp)
	{
		return EBTNodeResult::Failed;
	}

	if(IsSearchingKey.SelectedKeyName.IsValid())
	{
		BlackboardComp->SetValueAsBool(IsSearchingKey.SelectedKeyName, false);
	}

	if(LastSeenLocationKey.SelectedKeyName.IsValid())
	{
		BlackboardComp->ClearValue(LastSeenLocationKey.SelectedKeyName);
	}

	if(MoveToLocationKey.SelectedKeyName.IsValid())
	{
		BlackboardComp->ClearValue(MoveToLocationKey.SelectedKeyName);
	}
	
	if(TargetActorKey.SelectedKeyName.IsValid())
	{
		BlackboardComp->ClearValue(TargetActorKey.SelectedKeyName);
	}

	return EBTNodeResult::Succeeded;
}
