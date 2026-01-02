//KSJ : AO_STEval_AIBaseContext

#include "AI/StateTree/Evaluator/AO_STEval_AIBaseContext.h"
#include "AI/Base/AO_AICharacterBase.h"
#include "AI/Controller/AO_AIControllerBase.h"
#include "AI/Component/AO_AIMemoryComponent.h"
#include "Character/AO_PlayerCharacter.h"
#include "StateTreeExecutionContext.h"

void FAO_STEval_AIBaseContext::TreeStart(FStateTreeExecutionContext& Context) const
{
	FAO_STEval_AIBaseContext_InstanceData& InstanceData = Context.GetInstanceData<FAO_STEval_AIBaseContext_InstanceData>(*this);
	
	// 초기화
	InstanceData.bHasPlayerInSight = false;
	InstanceData.bIsStunned = false;
	InstanceData.NearestPlayerDistance = MAX_FLT;
	InstanceData.NearestPlayer = nullptr;
	InstanceData.LastHeardLocation = FVector::ZeroVector;
	InstanceData.bHasHeardSound = false;
}

void FAO_STEval_AIBaseContext::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FAO_STEval_AIBaseContext_InstanceData& InstanceData = Context.GetInstanceData<FAO_STEval_AIBaseContext_InstanceData>(*this);
	UpdateContextData(Context, InstanceData);
}

AAO_AICharacterBase* FAO_STEval_AIBaseContext::GetAICharacter(FStateTreeExecutionContext& Context) const
{
	if (UObject* OwnerObj = Context.GetOwner())
	{
		if (AActor* Owner = Cast<AActor>(OwnerObj))
		{
			return Cast<AAO_AICharacterBase>(Owner);
		}
	}
	return nullptr;
}

AAO_AIControllerBase* FAO_STEval_AIBaseContext::GetAIController(FStateTreeExecutionContext& Context) const
{
	AAO_AICharacterBase* AIChar = GetAICharacter(Context);
	if (!AIChar)
	{
		return nullptr;
	}

	return Cast<AAO_AIControllerBase>(AIChar->GetController());
}

void FAO_STEval_AIBaseContext::UpdateContextData(FStateTreeExecutionContext& Context, FAO_STEval_AIBaseContext_InstanceData& InstanceData) const
{
	AAO_AICharacterBase* AIChar = GetAICharacter(Context);
	AAO_AIControllerBase* AIController = GetAIController(Context);

	if (!AIChar || !AIController)
	{
		return;
	}

	// 기절 상태 확인
	InstanceData.bIsStunned = AIChar->IsStunned();

	// 시야 내 플레이어 확인
	InstanceData.bHasPlayerInSight = AIController->HasPlayerInSight();

	// 가장 가까운 플레이어 찾기
	InstanceData.NearestPlayer = AIController->GetNearestPlayerInSight();
	if (InstanceData.NearestPlayer.IsValid())
	{
		const float Dist = FVector::Dist(
			AIChar->GetActorLocation(),
			InstanceData.NearestPlayer->GetActorLocation()
		);
		InstanceData.NearestPlayerDistance = Dist;
	}
	else
	{
		InstanceData.NearestPlayerDistance = MAX_FLT;
	}

	// 소리 감지 확인
	UAO_AIMemoryComponent* Memory = AIChar->GetMemoryComponent();
	if (Memory)
	{
		FVector HeardLocation = Memory->GetLastHeardLocation();
		if (!HeardLocation.IsZero())
		{
			InstanceData.LastHeardLocation = HeardLocation;
			InstanceData.bHasHeardSound = true;
		}
		else
		{
			InstanceData.bHasHeardSound = false;
		}
	}
}

