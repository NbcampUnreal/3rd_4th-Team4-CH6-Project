// AO_STCond_IsKidnapping.cpp

#include "AI/StateTree/Condition/AO_STCond_IsKidnapping.h"
#include "AI/Character/AO_Insect.h"
#include "AIController.h"
#include "StateTreeExecutionContext.h"

bool FAO_STCond_IsKidnapping::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	
	AAO_Insect* Insect = Cast<AAO_Insect>(Context.GetOwner());
	if (!Insect)
	{
		if (AAIController* AIC = Cast<AAIController>(Context.GetOwner()))
		{
			Insect = Cast<AAO_Insect>(AIC->GetPawn());
		}
	}

	bool bIsKidnapping = Insect && Insect->IsKidnapping();
	return InstanceData.bInvert ? !bIsKidnapping : bIsKidnapping;
}

