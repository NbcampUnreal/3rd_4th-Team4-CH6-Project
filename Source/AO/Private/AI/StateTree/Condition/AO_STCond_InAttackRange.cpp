// AO_STCond_InAttackRange.cpp

#include "AI/StateTree/Condition/AO_STCond_InAttackRange.h"
#include "AI/Base/AO_AggressiveAIBase.h"
#include "AI/Controller/AO_AggressiveAICtrl.h"
#include "StateTreeExecutionContext.h"

bool FAO_STCond_InAttackRange::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FAO_STCond_InAttackRange_InstanceData& InstanceData = Context.GetInstanceData<FAO_STCond_InAttackRange_InstanceData>(*this);

	AAO_AggressiveAICtrl* Controller = GetAggressiveController(Context);
	if (!Controller)
	{
		return InstanceData.bInvert;
	}

	AAO_AggressiveAIBase* AI = Controller->GetAggressiveAI();
	if (!AI)
	{
		return InstanceData.bInvert;
	}

	const bool bInRange = AI->IsTargetInAttackRange();
	return InstanceData.bInvert ? !bInRange : bInRange;
}

AAO_AggressiveAICtrl* FAO_STCond_InAttackRange::GetAggressiveController(FStateTreeExecutionContext& Context) const
{
	if (AActor* Owner = Cast<AActor>(Context.GetOwner()))
	{
		if (AAO_AggressiveAICtrl* Controller = Cast<AAO_AggressiveAICtrl>(Owner))
		{
			return Controller;
		}
		if (APawn* Pawn = Cast<APawn>(Owner))
		{
			return Cast<AAO_AggressiveAICtrl>(Pawn->GetController());
		}
	}
	return nullptr;
}

