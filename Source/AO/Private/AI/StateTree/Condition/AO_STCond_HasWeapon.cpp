// AO_STCond_HasWeapon.cpp

#include "AI/StateTree/Condition/AO_STCond_HasWeapon.h"
#include "AI/Character/AO_Troll.h"
#include "AI/Controller/AO_TrollController.h"
#include "StateTreeExecutionContext.h"
#include "AO_Log.h"  // 추가

bool FAO_STCond_HasWeapon::TestCondition(FStateTreeExecutionContext& Context) const
{
	// [DEBUG] 함수 호출 여부 확인
	AO_LOG(LogKSJ, Warning, TEXT("[DEBUG] STCond_HasWeapon::TestCondition CALLED"));

	const FAO_STCond_HasWeapon_InstanceData& InstanceData = Context.GetInstanceData<FAO_STCond_HasWeapon_InstanceData>(*this);

	AAO_TrollController* Controller = GetTrollController(Context);
	if (!Controller)
	{
		AO_LOG(LogKSJ, Warning, TEXT("[DEBUG] STCond_HasWeapon: Controller is null, returning %s"), InstanceData.bInvert ? TEXT("true") : TEXT("false"));
		return InstanceData.bInvert;
	}

	AAO_Troll* Troll = Controller->GetTroll();
	if (!Troll)
	{
		AO_LOG(LogKSJ, Warning, TEXT("[DEBUG] STCond_HasWeapon: Troll is null, returning %s"), InstanceData.bInvert ? TEXT("true") : TEXT("false"));
		return InstanceData.bInvert;
	}

	const bool bHasWeapon = Troll->HasWeapon();
	const bool bResult = InstanceData.bInvert ? !bHasWeapon : bHasWeapon;
	
	AO_LOG(LogKSJ, Log, TEXT("[DEBUG] STCond_HasWeapon: bHasWeapon=%s, bInvert=%s, Result=%s"), 
		bHasWeapon ? TEXT("true") : TEXT("false"),
		InstanceData.bInvert ? TEXT("true") : TEXT("false"),
		bResult ? TEXT("true") : TEXT("false"));
	
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

