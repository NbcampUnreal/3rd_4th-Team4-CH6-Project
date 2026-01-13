//KSJ : AO_STCond_CeilingAvailable

#include "AI/StateTree/Condition/AO_STCond_CeilingAvailable.h"
#include "AI/Character/AO_Stalker.h"
#include "AI/Component/AO_CeilingMoveComponent.h"
#include "StateTreeExecutionContext.h"
#include "AIController.h"

bool FAO_STCond_CeilingAvailable::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	
	AActor* Owner = Cast<AActor>(Context.GetOwner());
	if (!Owner)
	{
		return false;
	}
	
	AAO_Stalker* Stalker = nullptr;
	
	if (APawn* Pawn = Cast<APawn>(Owner))
	{
		Stalker = Cast<AAO_Stalker>(Pawn);
	}
	else if (AController* Ctrl = Cast<AController>(Owner))
	{
		APawn* ControlledPawn = Ctrl->GetPawn();
		if (ControlledPawn)
		{
			Stalker = Cast<AAO_Stalker>(ControlledPawn);
		}
	}

	if (!Stalker)
	{
		return false;
	}

	UAO_CeilingMoveComponent* Comp = Stalker->GetCeilingMoveComponent();
	if (!Comp)
	{
		return false;
	}

	bool bCeilingAvailable = Comp->CheckCeilingAvailability();
	bool bResult = InstanceData.bInvert ? !bCeilingAvailable : bCeilingAvailable;
	
	return bResult;
}
