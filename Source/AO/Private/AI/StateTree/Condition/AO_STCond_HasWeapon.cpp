//KSJ : AO_STCond_HasWeapon

#include "AI/StateTree/Condition/AO_STCond_HasWeapon.h"
#include "AI/Character/AO_Troll.h"
#include "AI/Controller/AO_TrollController.h"
#include "StateTreeExecutionContext.h"

bool FAO_STCond_HasWeapon::TestCondition(FStateTreeExecutionContext& Context) const
{

	const FAO_STCond_HasWeapon_InstanceData& InstanceData = Context.GetInstanceData<FAO_STCond_HasWeapon_InstanceData>(*this);

	AAO_TrollController* Controller = GetTrollController(Context);
	if (!Controller)
	{
		return InstanceData.bInvert;
	}

	AAO_Troll* Troll = Controller->GetTroll();
	if (!Troll)
	{
		return InstanceData.bInvert;
	}

	const bool bHasWeapon = Troll->HasWeapon();
	const bool bResult = InstanceData.bInvert ? !bHasWeapon : bHasWeapon;
	
	
	return bResult;
}

AAO_TrollController* FAO_STCond_HasWeapon::GetTrollController(FStateTreeExecutionContext& Context) const
{
	if (AActor* Owner = Cast<AActor>(Context.GetOwner()))
	{
		if (AAO_TrollController* Controller = Cast<AAO_TrollController>(Owner))
		{
			return Controller;
		}
		if (APawn* Pawn = Cast<APawn>(Owner))
		{
			return Cast<AAO_TrollController>(Pawn->GetController());
		}
	}
	return nullptr;
}

