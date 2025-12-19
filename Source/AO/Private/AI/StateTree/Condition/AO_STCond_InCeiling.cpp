// AO_STCond_InCeiling.cpp

#include "AI/StateTree/Condition/AO_STCond_InCeiling.h"
#include "AI/Character/AO_Stalker.h"
#include "AI/Component/AO_CeilingMoveComponent.h"
#include "StateTreeExecutionContext.h"
#include "AIController.h"

bool FAO_STCond_InCeiling::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	
	AAO_Stalker* Stalker = nullptr;
	AActor* Owner = Cast<AActor>(Context.GetOwner());
	
	if (APawn* Pawn = Cast<APawn>(Owner))
	{
		Stalker = Cast<AAO_Stalker>(Pawn);
	}
	else if (AController* Ctrl = Cast<AController>(Owner))
	{
		Stalker = Cast<AAO_Stalker>(Ctrl->GetPawn());
	}

	bool bInCeiling = false;
	if (Stalker)
	{
		if (UAO_CeilingMoveComponent* Comp = Stalker->GetCeilingMoveComponent())
		{
			bInCeiling = Comp->IsInCeilingMode();
		}
	}

	return InstanceData.bInvert ? !bInCeiling : bInCeiling;
}

