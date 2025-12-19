// AO_STTask_Insect_Drag.cpp

#include "AI/StateTree/Task/AO_STTask_Insect_Drag.h"
#include "AI/Character/AO_Insect.h"
#include "AI/Controller/AO_InsectController.h"
#include "AI/Component/AO_KidnapComponent.h"
#include "Character/AO_PlayerCharacter.h"
#include "AIController.h"
#include "StateTreeExecutionContext.h"
#include "Navigation/PathFollowingComponent.h"

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

	// Victim을 바라보도록 포커스 설정 (뒷걸음질 효과)
	if (AActor* Victim = Insect->GetKidnapComponent()->GetCurrentVictim())
	{
		AIC->SetFocus(Victim);
	}

	// 초기 이동 명령
	AIC->MoveToLocation(InstanceData.SafeLocation, InstanceData.AcceptanceRadius, false, true, true, true, 0, false);

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FAO_STTask_Insect_Drag::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	AAIController* AIC = Cast<AAIController>(Context.GetOwner());
	if (!AIC) return EStateTreeRunStatus::Failed;

	AAO_Insect* Insect = Cast<AAO_Insect>(AIC->GetPawn());
	if (!Insect || !Insect->IsKidnapping())
	{
		return EStateTreeRunStatus::Failed; // 납치 중단됨
	}

	// 도착 확인
	if (AIC->GetMoveStatus() == EPathFollowingStatus::Idle)
	{
		// 도착했으면 납치 해제 및 던지기 수행
		// (Task 종료 후 State Tree의 다음 상태에서 처리할 수도 있지만, 여기서 처리하는 게 깔끔)
		Insect->GetKidnapComponent()->ReleaseKidnap(true); // 던지기 true
		return EStateTreeRunStatus::Succeeded;
	}

	// 목표 지점 갱신 (Evaluator가 SafeLocation을 계속 업데이트한다고 가정)
	// 매 프레임 MoveTo 호출은 비효율적이므로 거리 차이가 클 때만 업데이트하거나 PathFollowingComp 사용
	if (AIC->GetPathFollowingComponent())
	{
		FVector CurrentTarget = AIC->GetPathFollowingComponent()->GetPathDestination();
		if (FVector::DistSquared(CurrentTarget, InstanceData.SafeLocation) > 100.f * 100.f)
		{
			AIC->MoveToLocation(InstanceData.SafeLocation, InstanceData.AcceptanceRadius, false, true, true, true, 0, false);
		}
	}

	return EStateTreeRunStatus::Running;
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

