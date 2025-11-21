// AO_BTS_SyncHasItem.cpp

#include "AO/Public/AI/BT/Services/AO_BTS_SyncHasItem.h"
#include "AO/Public/AI/Character/AO_Enemy_Crab.h"
#include "AO/Public/AI/Item/AO_TestPickupItem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UAO_BTS_SyncHasItem::UAO_BTS_SyncHasItem()
{
	NodeName = TEXT("Sync bHasItem from Pawn");
	bNotifyTick = true;

	HasItemKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UAO_BTS_SyncHasItem, HasItemKey));
	TargetItemKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UAO_BTS_SyncHasItem, TargetItemKey), AAO_TestPickupItem::StaticClass());
}

void UAO_BTS_SyncHasItem::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	const AAIController* AICon = OwnerComp.GetAIOwner();
	AAO_Enemy_Crab* Pawn = AICon ? Cast<AAO_Enemy_Crab>(AICon->GetPawn()) : nullptr;

	if (BB == nullptr || Pawn == nullptr)
	{
		return;
	}

	const bool bHasItem = (Pawn->HeldItem != nullptr);
	BB->SetValueAsBool(HasItemKey.SelectedKeyName, bHasItem);

	if (bHasItem && TargetItemKey.SelectedKeyName != NAME_None)
	{
		// 이미 들고 있으면 새로운 TargetItem은 비워 중복 집기 방지
		BB->SetValueAsObject(TargetItemKey.SelectedKeyName, nullptr);
	}
}
