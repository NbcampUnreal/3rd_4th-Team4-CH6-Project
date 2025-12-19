// AO_STCond_CanAmbush.cpp

#include "AI/StateTree/Condition/AO_STCond_CanAmbush.h"
#include "AI/Controller/AO_AggressiveAICtrl.h"
#include "Character/AO_PlayerCharacter.h"
#include "StateTreeExecutionContext.h"

bool FAO_STCond_CanAmbush::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FAO_STCond_CanAmbush_InstanceData& InstanceData = Context.GetInstanceData<FAO_STCond_CanAmbush_InstanceData>(*this);

	AAO_AggressiveAICtrl* Ctrl = GetAggressiveController(Context);
	if (!Ctrl)
	{
		return InstanceData.bInvert;
	}

	// ChaseTarget이 없으면 시야 내 가장 가까운 플레이어 사용
	AAO_PlayerCharacter* Target = Ctrl->GetChaseTarget();
	if (!Target)
	{
		Target = Ctrl->GetNearestPlayerInSight();
	}

	APawn* StalkerPawn = Ctrl->GetPawn();
	if (!Target || !StalkerPawn)
	{
		return InstanceData.bInvert;
	}

	// 플레이어에서 Stalker로의 방향 벡터
	FVector DirToStalker = (StalkerPawn->GetActorLocation() - Target->GetActorLocation()).GetSafeNormal();
	
	// 플레이어의 전방 벡터와의 내적 계산
	float Dot = FVector::DotProduct(Target->GetActorForwardVector(), DirToStalker);

	// 등 뒤 또는 옆에 있는지 확인
	// Dot < BackAngleCosine: 등 뒤 (예: -0.3 = 110도)
	// Dot < SideAngleCosine: 옆 (예: 0.0 = 90도)
	bool bCanAmbush = (Dot <= InstanceData.BackAngleCosine) || 
	                  (Dot <= InstanceData.SideAngleCosine && Dot > InstanceData.BackAngleCosine);

	return InstanceData.bInvert ? !bCanAmbush : bCanAmbush;
}

AAO_AggressiveAICtrl* FAO_STCond_CanAmbush::GetAggressiveController(FStateTreeExecutionContext& Context) const
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

