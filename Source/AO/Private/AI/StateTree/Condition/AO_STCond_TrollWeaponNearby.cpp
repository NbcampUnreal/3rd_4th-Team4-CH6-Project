//KSJ : AO_STCond_TrollWeaponNearby

#include "AI/StateTree/Condition/AO_STCond_TrollWeaponNearby.h"
#include "AI/Character/AO_Troll.h"
#include "AI/Component/AO_WeaponHolderComp.h"
#include "AI/Controller/AO_TrollController.h"
#include "AI/Item/AO_TrollWeapon.h"
#include "StateTreeExecutionContext.h"

bool FAO_STCond_TrollWeaponNearby::TestCondition(FStateTreeExecutionContext& Context) const
{

	const FAO_STCond_TrollWeaponNearby_InstanceData& InstanceData = Context.GetInstanceData<FAO_STCond_TrollWeaponNearby_InstanceData>(*this);

	AAO_Troll* Troll = GetTroll(Context);
	if (!Troll)
	{
		return InstanceData.bInvert;
	}

	// 이미 무기를 들고 있으면 False (주울 필요 없음)
	if (Troll->HasWeapon())
	{
		return InstanceData.bInvert;
	}

	bool bWeaponNearby = false;
	UAO_WeaponHolderComp* WeaponHolder = Troll->GetWeaponHolderComponent();
	
	if (WeaponHolder)
	{
		// 1. 시야 감지 우선 (가장 빠름)
		AAO_TrollWeapon* FoundWeapon = WeaponHolder->FindNearestWeaponInSight();
		if (FoundWeapon)
		{
		}

		// 2. 시야에 없다면 거리 기반 검색 (옵션)
		if (!FoundWeapon && InstanceData.SearchRadius > 0.f)
		{
			FoundWeapon = WeaponHolder->FindNearestWeaponInRadius(InstanceData.SearchRadius);
			
		}

		bWeaponNearby = (FoundWeapon != nullptr);
	}
	else
	{
	}

	return InstanceData.bInvert ? !bWeaponNearby : bWeaponNearby;
}

AAO_Troll* FAO_STCond_TrollWeaponNearby::GetTroll(FStateTreeExecutionContext& Context) const
{
	AActor* Owner = Cast<AActor>(Context.GetOwner());
	if (!Owner)
	{
		return nullptr;
	}

	if (AAO_Troll* Troll = Cast<AAO_Troll>(Owner))
	{
		return Troll;
	}
	
	if (AAO_TrollController* Controller = Cast<AAO_TrollController>(Owner))
	{
		return Controller->GetTroll();
	}
	
	if (APawn* Pawn = Cast<APawn>(Owner))
	{
		if (AAO_TrollController* Controller = Cast<AAO_TrollController>(Pawn->GetController()))
		{
			return Controller->GetTroll();
		}
		return Cast<AAO_Troll>(Pawn);
	}

	return nullptr;
}

