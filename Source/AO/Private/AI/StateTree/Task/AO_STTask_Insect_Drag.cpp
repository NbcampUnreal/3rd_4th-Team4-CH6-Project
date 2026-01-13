//KSJ : AO_STTask_Insect_Drag

#include "AI/StateTree/Task/AO_STTask_Insect_Drag.h"
#include "AI/Character/AO_Insect.h"
#include "AI/Controller/AO_InsectController.h"
#include "AI/Component/AO_KidnapComponent.h"
#include "Character/AO_PlayerCharacter.h"
#include "AIController.h"
#include "StateTreeExecutionContext.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"

EStateTreeRunStatus FAO_STTask_Insect_Drag::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	AAIController* AIC = Cast<AAIController>(Context.GetOwner());
	
	if (!AIC)
	{
		return EStateTreeRunStatus::Failed;
	}

	AAO_Insect* Insect = Cast<AAO_Insect>(AIC->GetPawn());
	if (!Insect || !Insect->IsKidnapping())
	{
		return EStateTreeRunStatus::Failed;
	}

	// NavMesh 경로 기반 이동 시작 (벽 회피)
	FAIMoveRequest MoveReq;
	MoveReq.SetGoalLocation(InstanceData.SafeLocation);
	MoveReq.SetAcceptanceRadius(InstanceData.AcceptanceRadius);
	MoveReq.SetCanStrafe(false);
	
	FPathFollowingRequestResult MoveResult = AIC->MoveTo(MoveReq);
	if (MoveResult.Code != EPathFollowingRequestResult::RequestSuccessful)
	{
		return EStateTreeRunStatus::Failed;
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FAO_STTask_Insect_Drag::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	AAIController* AIC = Cast<AAIController>(Context.GetOwner());
	if (!AIC)
	{
		return EStateTreeRunStatus::Failed;
	}

	AAO_Insect* Insect = Cast<AAO_Insect>(AIC->GetPawn());
	if (!Insect || !Insect->IsKidnapping())
	{
		return EStateTreeRunStatus::Failed; // 납치 중단됨
	}

	AActor* Victim = Insect->GetKidnapComponent()->GetCurrentVictim();
	if (!Victim)
	{
		return EStateTreeRunStatus::Failed;
	}

	// MoveToLocation의 상태 확인
	EPathFollowingStatus::Type MoveStatus = AIC->GetMoveStatus();
	
	if (MoveStatus == EPathFollowingStatus::Idle)
	{
		// 도착 완료
		Insect->GetKidnapComponent()->ReleaseKidnap(true);
		return EStateTreeRunStatus::Succeeded;
	}
	
	if (MoveStatus == EPathFollowingStatus::Moving || MoveStatus == EPathFollowingStatus::Waiting)
	{
		// 이동 중 또는 경로 계산 대기 중
		return EStateTreeRunStatus::Running;
	}
	
	// 실패 상태
	return EStateTreeRunStatus::Failed;
}

void FAO_STTask_Insect_Drag::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	AAIController* AIC = Cast<AAIController>(Context.GetOwner());
	if (AIC)
	{
		AIC->ClearFocus(EAIFocusPriority::Gameplay);
		AIC->StopMovement();
	}
}

