// AO_BTT_ClearPackState.cpp

#include "AI/BT/BTTasks/AO_BTT_ClearPackState.h"

#include "BehaviorTree/BlackboardComponent.h"

UAO_BTT_ClearPackState::UAO_BTT_ClearPackState()
{
	NodeName = TEXT("Clear Pack State");

	IsPackNotifiedKey.AddBoolFilter(
		this,
		GET_MEMBER_NAME_CHECKED(UAO_BTT_ClearPackState, IsPackNotifiedKey)
	);

	IsInPackPositionKey.AddBoolFilter(
		this,
		GET_MEMBER_NAME_CHECKED(UAO_BTT_ClearPackState, IsInPackPositionKey)
	);

	ShouldStartPackAttackKey.AddBoolFilter(
		this,
		GET_MEMBER_NAME_CHECKED(UAO_BTT_ClearPackState, ShouldStartPackAttackKey)
	);

	IsHowlSourceKey.AddBoolFilter(
		this,
		GET_MEMBER_NAME_CHECKED(UAO_BTT_ClearPackState, IsHowlSourceKey)
	);
}

EBTNodeResult::Type UAO_BTT_ClearPackState::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory
)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if(!BlackboardComp)
	{
		return EBTNodeResult::Failed;
	}

	if(IsPackNotifiedKey.SelectedKeyName.IsValid())
	{
		BlackboardComp->SetValueAsBool(IsPackNotifiedKey.SelectedKeyName, false);
	}

	if(IsInPackPositionKey.SelectedKeyName.IsValid())
	{
		BlackboardComp->SetValueAsBool(IsInPackPositionKey.SelectedKeyName, false);
	}

	if(ShouldStartPackAttackKey.SelectedKeyName.IsValid())
	{
		BlackboardComp->SetValueAsBool(ShouldStartPackAttackKey.SelectedKeyName, false);
	}

	if(IsHowlSourceKey.SelectedKeyName.IsValid())
	{
		BlackboardComp->SetValueAsBool(IsHowlSourceKey.SelectedKeyName, false);
	}

	return EBTNodeResult::Succeeded;
}
