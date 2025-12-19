// AO_STEval_AggressiveCtx.cpp

#include "AI/StateTree/Evaluator/AO_STEval_AggressiveCtx.h"
#include "AI/Base/AO_AggressiveAIBase.h"
#include "AI/Controller/AO_AggressiveAICtrl.h"
#include "Character/AO_PlayerCharacter.h"
#include "StateTreeExecutionContext.h"
#include "AO_Log.h"

void FAO_STEval_AggressiveCtx::TreeStart(FStateTreeExecutionContext& Context) const
{
	FAO_STEval_AggressiveCtx_InstanceData& InstanceData = Context.GetInstanceData<FAO_STEval_AggressiveCtx_InstanceData>(*this);
	UpdateContextData(Context, InstanceData);
}

void FAO_STEval_AggressiveCtx::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FAO_STEval_AggressiveCtx_InstanceData& InstanceData = Context.GetInstanceData<FAO_STEval_AggressiveCtx_InstanceData>(*this);
	UpdateContextData(Context, InstanceData);
}

AAO_AggressiveAIBase* FAO_STEval_AggressiveCtx::GetAggressiveAI(FStateTreeExecutionContext& Context) const
{
	AAO_AggressiveAICtrl* Controller = GetAggressiveController(Context);
	if (Controller)
	{
		return Controller->GetAggressiveAI();
	}
	return nullptr;
}

AAO_AggressiveAICtrl* FAO_STEval_AggressiveCtx::GetAggressiveController(FStateTreeExecutionContext& Context) const
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

void FAO_STEval_AggressiveCtx::UpdateContextData(FStateTreeExecutionContext& Context, FAO_STEval_AggressiveCtx_InstanceData& InstanceData) const
{
	AAO_AggressiveAIBase* AI = GetAggressiveAI(Context);
	AAO_AggressiveAICtrl* Controller = GetAggressiveController(Context);

	if (!AI || !Controller)
	{
		// [Debug] 컨트롤러나 AI를 못 가져오는지 확인
		/*AO_LOG(LogKSJ, Warning, TEXT("AO_STEval_AggressiveCtx: Failed to get AI(%s) or Controller(%s)"), 
			AI ? *AI->GetName() : TEXT("None"), 
			Controller ? *Controller->GetName() : TEXT("None"));*/
		return;
	}

	// [Debug] Evaluator 실행 및 데이터 확인
	/*AO_LOG(LogKSJ, Log, TEXT("AO_STEval_AggressiveCtx: HasSight=%d, NearestDist=%.1f"), 
		Controller->HasPlayerInSight(), 
		NearestPlayer ? FVector::Dist(AI->GetActorLocation(), NearestPlayer->GetActorLocation()) : -1.f);*/

	// 기본 상태 업데이트
	InstanceData.bIsStunned = AI->IsStunned();
	InstanceData.bIsChasing = AI->IsInChaseMode();
	InstanceData.bIsSearching = AI->IsInSearchMode();
	InstanceData.bHasPlayerInSight = Controller->HasPlayerInSight();
	InstanceData.bTargetInAttackRange = AI->IsTargetInAttackRange();

	// 현재 대상
	InstanceData.CurrentTarget = AI->GetCurrentTarget();

	// 마지막으로 플레이어를 본 위치
	InstanceData.LastKnownTargetLocation = Controller->GetLastKnownTargetLocation();

	// 가장 가까운 플레이어 거리 계산
	AAO_PlayerCharacter* NearestPlayer = Controller->GetNearestPlayerInSight();
	if (NearestPlayer)
	{
		InstanceData.NearestPlayerDistance = FVector::Dist(AI->GetActorLocation(), NearestPlayer->GetActorLocation());
	}
	else
	{
		InstanceData.NearestPlayerDistance = MAX_FLT;
	}
}

