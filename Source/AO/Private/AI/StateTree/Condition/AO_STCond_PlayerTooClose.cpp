// AO_STCond_PlayerTooClose.cpp

#include "AI/StateTree/Condition/AO_STCond_PlayerTooClose.h"
#include "AI/Controller/AO_AggressiveAICtrl.h"
#include "Character/AO_PlayerCharacter.h"
#include "StateTreeExecutionContext.h"

bool FAO_STCond_PlayerTooClose::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FAO_STCond_PlayerTooClose_InstanceData& InstanceData = Context.GetInstanceData<FAO_STCond_PlayerTooClose_InstanceData>(*this);

	AAO_AggressiveAICtrl* Ctrl = GetAggressiveController(Context);
	if (!Ctrl)
	{
		return InstanceData.bInvert;
	}

	AAO_PlayerCharacter* Target = Ctrl->GetChaseTarget();
	APawn* StalkerPawn = Ctrl->GetPawn();
	if (!Target || !StalkerPawn)
	{
		return InstanceData.bInvert;
	}

	// 거리 계산
	float Distance = FVector::Dist(StalkerPawn->GetActorLocation(), Target->GetActorLocation());
	bool bTooClose = Distance < InstanceData.MinSafeDistance;

	return InstanceData.bInvert ? !bTooClose : bTooClose;
}

AAO_AggressiveAICtrl* FAO_STCond_PlayerTooClose::GetAggressiveController(FStateTreeExecutionContext& Context) const
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

