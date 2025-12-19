// AO_STCond_IsRetreating.cpp

#include "AI/StateTree/Condition/AO_STCond_IsRetreating.h"
#include "AI/Character/AO_Stalker.h"
#include "StateTreeExecutionContext.h"
#include "AIController.h"

bool FAO_STCond_IsRetreating::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FAO_STCond_IsRetreating_InstanceData& InstanceData = Context.GetInstanceData<FAO_STCond_IsRetreating_InstanceData>(*this);

	AAO_Stalker* Stalker = GetStalker(Context);
	if (!Stalker)
	{
		return InstanceData.bInvert;
	}

	bool bIsRetreating = Stalker->IsRetreating();
	return InstanceData.bInvert ? !bIsRetreating : bIsRetreating;
}

AAO_Stalker* FAO_STCond_IsRetreating::GetStalker(FStateTreeExecutionContext& Context) const
{
	if (AActor* Owner = Cast<AActor>(Context.GetOwner()))
	{
		if (AAO_Stalker* Stalker = Cast<AAO_Stalker>(Owner))
		{
			return Stalker;
		}
		if (AAIController* Controller = Cast<AAIController>(Owner))
		{
			return Cast<AAO_Stalker>(Controller->GetPawn());
		}
	}
	return nullptr;
}

