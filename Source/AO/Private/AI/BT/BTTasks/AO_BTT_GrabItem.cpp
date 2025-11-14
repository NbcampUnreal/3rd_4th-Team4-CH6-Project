// AO_BTT_GrabItem.cpp

#include "AO/Public/AI/BT/BTTasks/AO_BTT_GrabItem.h"
#include "AO/Public/AI/Character/AO_Enemy_Crab.h"
#include "AO/Public/AI/Item/AO_TestPickupItem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UAO_BTT_GrabItem::UAO_BTT_GrabItem()
{
	NodeName = TEXT("Grab Item");

	TargetItemKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UAO_BTT_GrabItem, TargetItemKey), AAO_TestPickupItem::StaticClass());
	HasItemKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UAO_BTT_GrabItem, HasItemKey));
}

EBTNodeResult::Type UAO_BTT_GrabItem::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	const AAIController* AICon = OwnerComp.GetAIOwner();
	AAO_Enemy_Crab* Crab = AICon ? Cast<AAO_Enemy_Crab>(AICon->GetPawn()) : nullptr;
	if (!Crab || !BB) { return EBTNodeResult::Failed; }

	AAO_TestPickupItem* Item = Cast<AAO_TestPickupItem>(BB->GetValueAsObject(TargetItemKey.SelectedKeyName));
	if (!Item) { return EBTNodeResult::Failed; }

	// 이미 들고 있다면 성공 처리
	if (Crab->HeldItem != nullptr)
	{
		if (HasItemKey.SelectedKeyName != NAME_None) { BB->SetValueAsBool(HasItemKey.SelectedKeyName, true); }
		BB->ClearValue(TargetItemKey.SelectedKeyName);
		return EBTNodeResult::Succeeded;
	}

	// 집기 시도
	Crab->GrabItem(Item);

	const bool bAttachedToCrab = (Item->GetAttachParentActor() == Crab);

	if (Crab->HeldItem == Item && Item->bIsPickedUp && bAttachedToCrab)
	{
		if (HasItemKey.SelectedKeyName != NAME_None)
		{
			BB->SetValueAsBool(HasItemKey.SelectedKeyName, true);
		}
		BB->ClearValue(TargetItemKey.SelectedKeyName);
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}