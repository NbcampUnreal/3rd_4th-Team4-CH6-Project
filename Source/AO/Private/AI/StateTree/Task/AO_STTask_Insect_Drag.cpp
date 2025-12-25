// AO_STTask_Insect_Drag.cpp

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

	// Victim을 바라보도록 포커스 설정 (뒷걸음질 효과)
	AActor* Victim = Insect->GetKidnapComponent()->GetCurrentVictim();
	if (Victim)
	{
		AIC->SetFocus(Victim);
	}

	// 뒷걸음질 구현:
	// PathFollowing을 사용하지 않고 직접 이동 입력을 제어하여 뒷걸음질 구현
	// Insect는 Victim을 바라보면서(bOrientRotationToMovement = false),
	// SafeLocation 방향으로 이동하되, Forward와 반대면 자동으로 뒷걸음질

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

	// 뒷걸음질 구현:
	// PathFollowing을 사용하지 않고 직접 이동 입력을 제어하여 뒷걸음질 구현
	AActor* Victim = Insect->GetKidnapComponent()->GetCurrentVictim();
	if (!Victim)
	{
		return EStateTreeRunStatus::Failed;
	}

	FVector CurrentLocation = Insect->GetActorLocation();
	FVector ToSafeLocation = (InstanceData.SafeLocation - CurrentLocation);
	ToSafeLocation.Z = 0.f;
	float DistanceToSafe = ToSafeLocation.Size();
	
	// 도착 확인 (AcceptanceRadius 내에 있으면 도착)
	if (DistanceToSafe <= InstanceData.AcceptanceRadius)
	{
		// 도착했으면 납치 해제 및 던지기 수행
		Insect->GetKidnapComponent()->ReleaseKidnap(true); // 던지기 true
		return EStateTreeRunStatus::Succeeded;
	}
	
	// SafeLocation 방향 정규화
	ToSafeLocation.Normalize();
	
	// NavMesh 프로젝션으로 이동 가능한 방향 확인
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(AIC->GetWorld());
	if (NavSys)
	{
		FVector TestLocation = CurrentLocation + ToSafeLocation * 100.f;
		FNavLocation NavLocation;
		if (NavSys->ProjectPointToNavigation(TestLocation, NavLocation))
		{
			// NavMesh 위에 있으면 해당 방향으로 이동
			FVector NavDirection = (NavLocation.Location - CurrentLocation).GetSafeNormal();
			NavDirection.Z = 0.f;
			
			// SafeLocation 방향으로 이동 입력
			// Insect는 Victim을 바라보고 있으므로(bOrientRotationToMovement = false),
			// SafeLocation 방향이 Insect의 Forward와 반대면 자동으로 뒷걸음질
			Insect->AddMovementInput(NavDirection, 1.0f);
		}
		else
		{
			// NavMesh 위가 아니면 직접 방향으로 이동 시도
			Insect->AddMovementInput(ToSafeLocation, 1.0f);
		}
	}
	else
	{
		// NavMesh가 없으면 직접 방향으로 이동
		Insect->AddMovementInput(ToSafeLocation, 1.0f);
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

