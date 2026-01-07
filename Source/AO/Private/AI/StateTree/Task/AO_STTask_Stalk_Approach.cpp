//KSJ : AO_STTask_Stalk_Approach

#include "AI/StateTree/Task/AO_STTask_Stalk_Approach.h"
#include "AI/Controller/AO_StalkerController.h"
#include "AI/Character/AO_Stalker.h"
#include "AI/Component/AO_CeilingMoveComponent.h"
#include "Character/AO_PlayerCharacter.h"
#include "StateTreeExecutionContext.h"
#include "Navigation/PathFollowingComponent.h"

EStateTreeRunStatus FAO_STTask_Stalk_Approach::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	
	AActor* Owner = Cast<AActor>(Context.GetOwner());
	if (APawn* Pawn = Cast<APawn>(Owner))
	{
		InstanceData.Controller = Cast<AAO_StalkerController>(Pawn->GetController());
	}
	else if (AController* Ctrl = Cast<AController>(Owner))
	{
		InstanceData.Controller = Cast<AAO_StalkerController>(Ctrl);
	}

	if (!InstanceData.Controller)
	{
		return EStateTreeRunStatus::Failed;
	}

	AAO_PlayerCharacter* Target = InstanceData.Controller->GetChaseTarget();
	if (!Target)
	{
		return EStateTreeRunStatus::Failed;
	}

	// 플레이어가 이미 나를 보고 있으면 바로 실패 (Hide로 전환)
	if (InstanceData.Controller->IsPlayerLookingAtMe(Target, InstanceData.LookToleranceDegrees))
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.LookCheckTimer = 0.f;
	InstanceData.bIsMoving = false;
	InstanceData.bWaitingForTransition = false;

	// Stalker 및 Ceiling Component 캐싱
	InstanceData.Stalker = InstanceData.Controller->GetStalker();
	if (InstanceData.Stalker)
	{
		InstanceData.CeilingComp = InstanceData.Stalker->GetCeilingMoveComponent();
	}

	// 초기 거리 체크하여 천장 모드 설정
	if (InstanceData.Stalker && InstanceData.CeilingComp)
	{
		const float DistanceToTarget = FVector::Dist(InstanceData.Stalker->GetActorLocation(), Target->GetActorLocation());
		
		// 거리가 멀면 천장 모드 시도, 가까우면 바닥 모드
		if (DistanceToTarget > InstanceData.CeilingModeDistance)
		{
			// 천장이 있으면 천장 모드로 전환
			if (InstanceData.CeilingComp->CheckCeilingAvailability())
			{
				InstanceData.Stalker->PlayCeilingTransitionMontage(true);
			}
		}
		else
		{
			// 가까우면 바닥 모드로 전환
			InstanceData.Stalker->PlayCeilingTransitionMontage(false);
		}

		// 전환 몽타주가 재생되면, 종료 후 MoveTo를 시작한다.
		if (InstanceData.Stalker->IsTransitioningCeiling())
		{
			InstanceData.bWaitingForTransition = true;
		}
	}

	// 전환 중이면 이동을 잠시 보류
	if (!InstanceData.bWaitingForTransition)
	{
		// 타겟 위치로 이동 시작
		InstanceData.Controller->MoveToActor(Target, InstanceData.AttackRange * 0.5f);
		InstanceData.bIsMoving = true;
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FAO_STTask_Stalk_Approach::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	if (!InstanceData.Controller)
	{
		return EStateTreeRunStatus::Failed;
	}

	AAO_Stalker* Stalker = InstanceData.Controller->GetStalker();
	AAO_PlayerCharacter* Target = InstanceData.Controller->GetChaseTarget();

	if (!Stalker || !Target)
	{
		return EStateTreeRunStatus::Failed;
	}

	// 전환 대기 중: 몽타주가 끝나면 이동을 시작한다.
	if (InstanceData.bWaitingForTransition && InstanceData.Stalker)
	{
		if (InstanceData.Stalker->IsTransitioningCeiling())
		{
			return EStateTreeRunStatus::Running;
		}
		InstanceData.bWaitingForTransition = false;

		// 전환 완료 후 MoveTo 재시작
		InstanceData.Controller->MoveToActor(Target, InstanceData.AttackRange * 0.5f);
		InstanceData.bIsMoving = true;
	}

	// 시선 체크 (주기적으로)
	InstanceData.LookCheckTimer += DeltaTime;
	if (InstanceData.LookCheckTimer >= InstanceData.LookCheckInterval)
	{
		InstanceData.LookCheckTimer = 0.f;

		// 플레이어가 나를 보고 있으면 실패 (Hide로 전환)
		if (InstanceData.Controller->IsPlayerLookingAtMe(Target, InstanceData.LookToleranceDegrees))
		{
			return EStateTreeRunStatus::Failed;
		}
	}

	// 거리 체크
	const float DistanceToTarget = FVector::Dist(Stalker->GetActorLocation(), Target->GetActorLocation());
	
	// 공격 거리 도달 시 성공 (Attack으로 전환)
	if (DistanceToTarget <= InstanceData.AttackRange)
	{
		// 공격 전에 바닥으로 내려오기
		if (InstanceData.CeilingComp && InstanceData.CeilingComp->IsInCeilingMode())
		{
			InstanceData.Stalker->PlayCeilingTransitionMontage(false);
			if (InstanceData.Stalker && InstanceData.Stalker->IsTransitioningCeiling())
			{
				InstanceData.bWaitingForTransition = true;
				InstanceData.bIsMoving = false;
				return EStateTreeRunStatus::Running;
			}
		}
		return EStateTreeRunStatus::Succeeded;
	}

	// 거리 기반 천장 모드 전환
	if (InstanceData.CeilingComp)
	{
		if (DistanceToTarget > InstanceData.CeilingModeDistance)
		{
			// 멀면 천장 모드 시도 (천장이 있을 때만)
			if (!InstanceData.CeilingComp->IsInCeilingMode())
			{
				if (InstanceData.CeilingComp->CheckCeilingAvailability())
				{
					InstanceData.Stalker->PlayCeilingTransitionMontage(true);
					if (InstanceData.Stalker && InstanceData.Stalker->IsTransitioningCeiling())
					{
						InstanceData.bWaitingForTransition = true;
						InstanceData.bIsMoving = false;
						return EStateTreeRunStatus::Running;
					}
				}
			}
		}
		else
		{
			// 가까우면 바닥 모드로 전환
			if (InstanceData.CeilingComp->IsInCeilingMode())
			{
				InstanceData.Stalker->PlayCeilingTransitionMontage(false);
				if (InstanceData.Stalker && InstanceData.Stalker->IsTransitioningCeiling())
				{
					InstanceData.bWaitingForTransition = true;
					InstanceData.bIsMoving = false;
					return EStateTreeRunStatus::Running;
				}
			}
		}
	}

	// 이동 상태 확인 및 재이동
	EPathFollowingStatus::Type MoveStatus = InstanceData.Controller->GetMoveStatus();
	if (MoveStatus == EPathFollowingStatus::Idle && InstanceData.bIsMoving)
	{
		// 이동이 완료되었지만 아직 공격 거리가 아니면 다시 이동
		InstanceData.Controller->MoveToActor(Target, InstanceData.AttackRange * 0.5f);
	}

	return EStateTreeRunStatus::Running;
}

void FAO_STTask_Stalk_Approach::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	if (InstanceData.Controller)
	{
		InstanceData.Controller->StopMovement();
	}
}

