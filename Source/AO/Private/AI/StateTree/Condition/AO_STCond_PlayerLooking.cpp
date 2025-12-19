// AO_STCond_PlayerLooking.cpp

#include "AI/StateTree/Condition/AO_STCond_PlayerLooking.h"
#include "AI/Controller/AO_AggressiveAICtrl.h"
#include "Character/AO_PlayerCharacter.h"
#include "StateTreeExecutionContext.h"

bool FAO_STCond_PlayerLooking::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	
	AActor* Owner = Cast<AActor>(Context.GetOwner());
	AAO_AggressiveAICtrl* Ctrl = nullptr;
	if (APawn* Pawn = Cast<APawn>(Owner))
	{
		Ctrl = Cast<AAO_AggressiveAICtrl>(Pawn->GetController());
	}
	else
	{
		Ctrl = Cast<AAO_AggressiveAICtrl>(Owner);
	}

	bool bIsLooking = false;
	if (Ctrl)
	{
		// ChaseTarget이 없으면 시야 내 가장 가까운 플레이어 사용
		AAO_PlayerCharacter* Target = Ctrl->GetChaseTarget();
		if (!Target)
		{
			Target = Ctrl->GetNearestPlayerInSight();
		}

		if (Target && Ctrl->GetPawn())
		{
			FVector DirToAI = (Ctrl->GetPawn()->GetActorLocation() - Target->GetActorLocation()).GetSafeNormal();
			float Dot = FVector::DotProduct(Target->GetActorForwardVector(), DirToAI);
			
			if (Dot >= InstanceData.FOVCosine)
			{
				bIsLooking = true;
			}
		}
	}

	return InstanceData.bInvert ? !bIsLooking : bIsLooking;
}

