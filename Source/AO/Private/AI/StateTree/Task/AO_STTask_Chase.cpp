// AO_STTask_Chase.cpp

#include "AI/StateTree/Task/AO_STTask_Chase.h"
#include "AI/Base/AO_AggressiveAIBase.h"
#include "AI/Character/AO_Stalker.h"
#include "AI/Controller/AO_AggressiveAICtrl.h"
#include "AI/Controller/AO_AIControllerBase.h"
#include "Character/AO_PlayerCharacter.h"
#include "StateTreeExecutionContext.h"
#include "Navigation/PathFollowingComponent.h"
#include "AO_Log.h"
#include "AI/Component/AO_CeilingMoveComponent.h"

EStateTreeRunStatus FAO_STTask_Chase::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FAO_STTask_Chase_InstanceData& InstanceData = Context.GetInstanceData<FAO_STTask_Chase_InstanceData>(*this);
	InstanceData.PathUpdateTimer = 0.f;
	InstanceData.bIsMoving = false;

	AAO_AggressiveAICtrl* Controller = GetAggressiveController(Context);
	if (!Controller)
	{
		return EStateTreeRunStatus::Failed;
	}

	AAO_PlayerCharacter* Target = Controller->GetChaseTarget();
	if (!Target)
	{
		return EStateTreeRunStatus::Failed;
	}

	// 추격 시작
	if (StartChasing(Controller, Target, InstanceData.AcceptanceRadius))
	{
		InstanceData.bIsMoving = true;
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FAO_STTask_Chase::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FAO_STTask_Chase_InstanceData& InstanceData = Context.GetInstanceData<FAO_STTask_Chase_InstanceData>(*this);

	AAO_AggressiveAICtrl* Controller = GetAggressiveController(Context);
	if (!Controller)
	{
		return EStateTreeRunStatus::Failed;
	}

	AAO_AggressiveAIBase* AI = Controller->GetAggressiveAI();
	if (!AI)
	{
		return EStateTreeRunStatus::Failed;
	}

	// 기절 상태면 중단
	if (AI->IsStunned())
	{
		return EStateTreeRunStatus::Failed;
	}

	AAO_PlayerCharacter* Target = Controller->GetChaseTarget();
	if (!Target)
	{
		// 대상이 없으면 실패
		return EStateTreeRunStatus::Failed;
	}

	// 공격 범위에 도달했으면 성공
	if (AI->IsTargetInAttackRange())
	{
		return EStateTreeRunStatus::Succeeded;
	}

	// 경로 갱신 타이머
	InstanceData.PathUpdateTimer += DeltaTime;
	if (InstanceData.PathUpdateTimer >= InstanceData.PathUpdateInterval)
	{
		InstanceData.PathUpdateTimer = 0.f;
		StartChasing(Controller, Target, InstanceData.AcceptanceRadius);
	}

	// 더 가까운 플레이어로 대상 갱신 (Controller에서 처리)
	Controller->UpdateChaseTargetToNearest();

	// Stalker 전용: 천장 감지 및 조건부 천장 모드 전환
	// 플레이어 시야에 있으면 바닥 모드, 없으면 천장 모드 가능
	if (AAO_Stalker* Stalker = Cast<AAO_Stalker>(AI))
	{
		if (UAO_CeilingMoveComponent* CeilingComp = Stalker->GetCeilingMoveComponent())
		{
			// 플레이어가 Stalker를 보고 있는지 확인
			bool bPlayerCanSeeStalker = false;
			if (AAO_AIControllerBase* AICtrl = Cast<AAO_AIControllerBase>(Controller))
			{
				// 플레이어가 Stalker를 볼 수 있는지 체크 (간단한 LineTrace)
				FVector StalkerLocation = Stalker->GetActorLocation();
				FVector PlayerLocation = Target->GetActorLocation();
				FVector DirToStalker = (StalkerLocation - PlayerLocation).GetSafeNormal();
				
				// 플레이어의 시야각 체크 (플레이어가 Stalker 쪽을 보고 있는지)
				float Dot = FVector::DotProduct(Target->GetActorForwardVector(), DirToStalker);
				if (Dot > 0.3f) // 약 70도 이내
				{
					// LineTrace로 장애물 체크
					FHitResult Hit;
					FCollisionQueryParams Params;
					Params.AddIgnoredActor(Stalker);
					Params.AddIgnoredActor(Target);
					
					if (AI->GetWorld()->LineTraceSingleByChannel(Hit, PlayerLocation, StalkerLocation, ECC_Visibility, Params))
					{
						// 장애물이 있으면 플레이어가 볼 수 없음
						bPlayerCanSeeStalker = false;
					}
					else
					{
						// 장애물이 없으면 플레이어가 볼 수 있음
						bPlayerCanSeeStalker = true;
					}
				}
			}

			// 플레이어가 Stalker를 볼 수 있으면 바닥 모드 유지
			if (bPlayerCanSeeStalker)
			{
				if (CeilingComp->IsInCeilingMode())
				{
					Stalker->SetCeilingMode(false);
					AO_LOG(LogKSJ, Log, TEXT("Stalker: Disabled ceiling mode - player can see"));
				}
			}
			else
			{
				// 플레이어가 Stalker를 볼 수 없으면 천장 모드 가능
				if (!CeilingComp->IsInCeilingMode() && CeilingComp->CheckCeilingAvailability())
				{
					Stalker->SetCeilingMode(true);
					AO_LOG(LogKSJ, Log, TEXT("Stalker: Enabled ceiling mode - player cannot see"));
				}
			}
		}
	}

	return EStateTreeRunStatus::Running;
}

void FAO_STTask_Chase::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FAO_STTask_Chase_InstanceData& InstanceData = Context.GetInstanceData<FAO_STTask_Chase_InstanceData>(*this);

	if (InstanceData.bIsMoving)
	{
		AAO_AggressiveAICtrl* Controller = GetAggressiveController(Context);
		if (Controller)
		{
			Controller->StopMovement();
		}
		InstanceData.bIsMoving = false;
	}
}

AAO_AggressiveAICtrl* FAO_STTask_Chase::GetAggressiveController(FStateTreeExecutionContext& Context) const
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

bool FAO_STTask_Chase::StartChasing(AAO_AggressiveAICtrl* Controller, AAO_PlayerCharacter* Target, float AcceptRadius) const
{
	if (!Controller || !Target)
	{
		return false;
	}

	const FVector TargetLocation = Target->GetActorLocation();
	
	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalLocation(TargetLocation);
	MoveRequest.SetAcceptanceRadius(AcceptRadius);
	MoveRequest.SetUsePathfinding(true);

	const FPathFollowingRequestResult Result = Controller->MoveTo(MoveRequest);
	return Result.Code != EPathFollowingRequestResult::Failed;
}

