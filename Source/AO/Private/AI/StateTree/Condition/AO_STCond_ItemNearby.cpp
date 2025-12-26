// AO_STCond_ItemNearby.cpp

#include "AI/StateTree/Condition/AO_STCond_ItemNearby.h"
#include "AI/Character/AO_Crab.h"
#include "AI/Component/AO_ItemCarryComponent.h"
#include "Item/AO_MasterItem.h"
#include "AIController.h"
#include "StateTreeExecutionContext.h"
#include "AO_Log.h"

bool FAO_STCond_ItemNearby::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FAO_STCond_ItemNearby_InstanceData& InstanceData = Context.GetInstanceData<FAO_STCond_ItemNearby_InstanceData>(*this);

	// Crab이 유효해야 주변 아이템 확인 가능
	AAO_Crab* Crab = GetCrab(Context);
	if (!ensureMsgf(Crab, TEXT("STCond_ItemNearby: Crab is null")))
	{
		return InstanceData.bInvert;
	}

	bool bItemNearby = false;

	// ItemCarryComponent를 통해 주변 아이템 검색
	UAO_ItemCarryComponent* CarryComp = Crab->GetItemCarryComponent();
	if (CarryComp)
	{
		AAO_MasterItem* FoundItem = CarryComp->FindNearbyItem(InstanceData.SearchRadius);
		bItemNearby = FoundItem != nullptr;
	}

	return InstanceData.bInvert ? !bItemNearby : bItemNearby;
}

AAO_Crab* FAO_STCond_ItemNearby::GetCrab(FStateTreeExecutionContext& Context) const
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
