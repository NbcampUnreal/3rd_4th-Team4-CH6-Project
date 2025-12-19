// AO_STEval_InsectContext.cpp

#include "AI/StateTree/Evaluator/AO_STEval_InsectContext.h"
#include "AI/Character/AO_Insect.h"
#include "AIController.h"
#include "StateTreeExecutionContext.h"

void FAO_STEval_InsectContext::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	
	AAO_Insect* Insect = Cast<AAO_Insect>(Context.GetOwner());
	if (!Insect)
	{
		if (AAIController* AIC = Cast<AAIController>(Context.GetOwner()))
		{
			Insect = Cast<AAO_Insect>(AIC->GetPawn());
		}
	}

	if (Insect)
	{
		InstanceData.bIsKidnapping = Insect->IsKidnapping();
		
		if (InstanceData.bIsKidnapping)
		{
			// 납치 중일 때만 안전 위치 계산
			InstanceData.SafeLocation = Insect->CalculateSafeDropLocation();
		}
		else
		{
			InstanceData.SafeLocation = FVector::ZeroVector;
		}
	}
}

