// AO_STCond_PlayerInSight.cpp

#include "AI/StateTree/Condition/AO_STCond_PlayerInSight.h"

#include "AO_Log.h"
#include "AI/Controller/AO_AIControllerBase.h"
#include "StateTreeExecutionContext.h"

bool FAO_STCond_PlayerInSight::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FAO_STCond_PlayerInSight_InstanceData& InstanceData = Context.GetInstanceData<FAO_STCond_PlayerInSight_InstanceData>(*this);

	AAO_AIControllerBase* Controller = GetAIController(Context);
	if (!Controller)
	{
		return InstanceData.bInvert;
	}

	const bool bHasPlayerInSight = Controller->HasPlayerInSight();

	return InstanceData.bInvert ? !bHasPlayerInSight : bHasPlayerInSight;
}

AAO_AIControllerBase* FAO_STCond_PlayerInSight::GetAIController(FStateTreeExecutionContext& Context) const
{
	if (AActor* Owner = Cast<AActor>(Context.GetOwner()))
	{
		if (AAO_AIControllerBase* Controller = Cast<AAO_AIControllerBase>(Owner))
		{
			return Controller;
		}
		if (APawn* Pawn = Cast<APawn>(Owner))
		{
			return Cast<AAO_AIControllerBase>(Pawn->GetController());
		}
	}
	return nullptr;
}

