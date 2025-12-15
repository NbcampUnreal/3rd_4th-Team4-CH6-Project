// AO_STCond_HasItem.cpp

#include "AI/StateTree/Condition/AO_STCond_HasItem.h"
#include "AI/Character/AO_Crab.h"
#include "AIController.h"
#include "StateTreeExecutionContext.h"

bool FAO_STCond_HasItem::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FAO_STCond_HasItem_InstanceData& InstanceData = Context.GetInstanceData<FAO_STCond_HasItem_InstanceData>(*this);

	// Crab이 유효해야 아이템 소지 여부 확인 가능
	AAO_Crab* Crab = GetCrab(Context);
	if (!ensureMsgf(Crab, TEXT("STCond_HasItem: Crab is null")))
	{
		return InstanceData.bInvert;
	}

	bool bHasItem = Crab->IsCarryingItem();
	return InstanceData.bInvert ? !bHasItem : bHasItem;
}

AAO_Crab* FAO_STCond_HasItem::GetCrab(FStateTreeExecutionContext& Context) const
{
	if (AActor* Owner = Cast<AActor>(Context.GetOwner()))
	{
		if (AAO_Crab* Crab = Cast<AAO_Crab>(Owner))
		{
			return Crab;
		}
		if (AAIController* Controller = Cast<AAIController>(Owner))
		{
			return Cast<AAO_Crab>(Controller->GetPawn());
		}
	}
	return nullptr;
}
