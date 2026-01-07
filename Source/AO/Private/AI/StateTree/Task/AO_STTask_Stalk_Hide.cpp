//KSJ : AO_STTask_Stalk_Hide

#include "AI/StateTree/Task/AO_STTask_Stalk_Hide.h"
#include "AI/Controller/AO_StalkerController.h"
#include "AI/Character/AO_Stalker.h"
#include "AI/Component/AO_CeilingMoveComponent.h"
#include "Character/AO_PlayerCharacter.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "StateTreeExecutionContext.h"
#include "Navigation/PathFollowingComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"

EStateTreeRunStatus FAO_STTask_Stalk_Hide::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	
	AActor* Owner = Cast<AActor>(Context.GetOwner());
	if (AAO_StalkerController* Ctrl = Cast<AAO_StalkerController>(Owner ? Owner->GetInstigatorController() : nullptr))
	{
		InstanceData.Controller = Ctrl;
	}
	else if (APawn* Pawn = Cast<APawn>(Owner))
	{
		InstanceData.Controller = Cast<AAO_StalkerController>(Pawn->GetController());
	}

	if (!InstanceData.Controller)
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.PlayerProximityCheckTimer = 0.f;
	InstanceData.CurrentHideLocation = FVector::ZeroVector;
	InstanceData.bIsMoving = false;
	InstanceData.bAwaitingEQSResult = false;
	InstanceData.EQSWaitTimer = 0.f;
	InstanceData.LookingPlayer = nullptr;

	// 후퇴 모드인지 확인
	AAO_Stalker* Stalker = InstanceData.Controller->GetStalker();
	bool bIsRetreating = Stalker && Stalker->IsRetreating();

	// Hide 상태에서는 무조건 바닥 모드로 전환 (천장 사용 안 함)
	if (Stalker)
	{
		if (UAO_CeilingMoveComponent* CeilingComp = Stalker->GetCeilingMoveComponent())
		{
			if (CeilingComp->IsInCeilingMode())
			{
				Stalker->PlayCeilingTransitionMontage(false);
			}
		}
	}

	// 나를 보고 있는 플레이어 찾기 (뒷걸음질 대상)
	if (InstanceData.bEnableBackpedal && Stalker)
	{
		if (UWorld* World = Stalker->GetWorld())
		{
			TArray<AActor*> AllPlayers;
			UGameplayStatics::GetAllActorsOfClass(World, AAO_PlayerCharacter::StaticClass(), AllPlayers);

			float ClosestDistance = FLT_MAX;
			for (AActor* PlayerActor : AllPlayers)
			{
				if (InstanceData.Controller->IsPlayerLookingAtMe(PlayerActor))
				{
					const float Distance = FVector::Dist(Stalker->GetActorLocation(), PlayerActor->GetActorLocation());
					if (Distance < ClosestDistance)
					{
						ClosestDistance = Distance;
						InstanceData.LookingPlayer = PlayerActor;
					}
				}
			}
		}

		// 뒷걸음질 모드: 이동 방향으로 회전하지 않도록 설정
		if (InstanceData.LookingPlayer.IsValid())
		{
			if (UCharacterMovementComponent* MoveComp = Stalker->GetCharacterMovement())
			{
				MoveComp->bOrientRotationToMovement = false;
			}
			Stalker->bUseControllerRotationYaw = false;
		}
	}

	// 엄폐 위치 찾기 (EQS 또는 직접 계산)
	AAO_PlayerCharacter* Target = InstanceData.Controller->GetChaseTarget();
	FVector HideLocation = FVector::ZeroVector;

	if (bIsRetreating)
	{
		// 후퇴 모드: 플레이어로부터 멀어지고 시야가 가려지는 곳으로 이동
		HideLocation = InstanceData.Controller->FindRetreatLocation();
		if (!HideLocation.IsZero())
		{
			InstanceData.CurrentHideLocation = HideLocation;
			InstanceData.Controller->MoveToLocation(HideLocation);
			InstanceData.bIsMoving = true;
		}
	}
	else if (InstanceData.HideQuery)
	{
		// KSJ:
		// StateTree Task의 InstanceData를 비동기 콜백에서 참조 캡처하면
		// 상태 전이/재진입 타이밍에 따라 크래시/오동작 위험이 크다.
		// EQS 결과는 Controller에 저장하고, Task는 Tick에서 Consume하여 MoveTo를 시작한다.
		InstanceData.Controller->RequestHideLocationEQS(InstanceData.HideQuery);
		InstanceData.bAwaitingEQSResult = true;
	}
	else
	{
		// EQS가 없으면 Controller의 FindHideLocation 사용
		HideLocation = InstanceData.Controller->FindHideLocation(1000.f, Target);
		if (!HideLocation.IsZero())
		{
			InstanceData.CurrentHideLocation = HideLocation;
			InstanceData.Controller->MoveToLocation(HideLocation);
			InstanceData.bIsMoving = true;
		}
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FAO_STTask_Stalk_Hide::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	if (!InstanceData.Controller) return EStateTreeRunStatus::Failed;

	AAO_Stalker* Stalker = InstanceData.Controller->GetStalker();
	AAO_PlayerCharacter* Target = InstanceData.Controller->GetChaseTarget();

	if (!Stalker || !Target)
	{
		return EStateTreeRunStatus::Failed;
	}

	// EQS 결과 대기 중이면, 결과를 소비해서 이동을 시작한다.
	if (InstanceData.bAwaitingEQSResult && !InstanceData.bIsMoving)
	{
		InstanceData.EQSWaitTimer += DeltaTime;

		FVector EQSLocation = FVector::ZeroVector;
		if (InstanceData.Controller->ConsumePendingHideLocation(EQSLocation))
		{
			InstanceData.CurrentHideLocation = EQSLocation;
			InstanceData.Controller->MoveToLocation(EQSLocation);
			InstanceData.bIsMoving = true;
			InstanceData.bAwaitingEQSResult = false;
		}
		else if (InstanceData.EQSWaitTimer >= InstanceData.EQSWaitTimeout)
		{
			// 타임아웃 시 동기 방식으로 폴백
			InstanceData.bAwaitingEQSResult = false;
			FVector Fallback = InstanceData.Controller->FindHideLocation(1000.f, Target);
			if (!Fallback.IsZero())
			{
				InstanceData.CurrentHideLocation = Fallback;
				InstanceData.Controller->MoveToLocation(Fallback);
				InstanceData.bIsMoving = true;
			}
		}
	}

	// 뒷걸음질: 나를 보고 있는 플레이어를 바라보며 이동
	if (InstanceData.bEnableBackpedal && InstanceData.LookingPlayer.IsValid() && InstanceData.bIsMoving)
	{
		AActor* LookingPlayerActor = InstanceData.LookingPlayer.Get();
		if (LookingPlayerActor)
		{
			// 플레이어를 향한 방향 계산
			FVector ToPlayer = LookingPlayerActor->GetActorLocation() - Stalker->GetActorLocation();
			ToPlayer.Z = 0.f; // 수평 방향만 고려
			
			if (!ToPlayer.IsNearlyZero())
			{
				FRotator TargetRotation = ToPlayer.Rotation();
				FRotator CurrentRotation = Stalker->GetActorRotation();
				
				// 부드러운 회전 보간
				FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, InstanceData.BackpedalRotationSpeed / 90.f);
				Stalker->SetActorRotation(FRotator(0.f, NewRotation.Yaw, 0.f));
			}
		}
	}

	// 플레이어 접근 체크 (주기적으로)
	InstanceData.PlayerProximityCheckTimer += DeltaTime;
	const float ProximityCheckInterval = 0.5f; // 0.5초마다 체크

	if (InstanceData.PlayerProximityCheckTimer >= ProximityCheckInterval)
	{
		InstanceData.PlayerProximityCheckTimer = 0.f;

		// 플레이어와의 거리 체크
		const float DistanceToPlayer = FVector::Dist(Stalker->GetActorLocation(), Target->GetActorLocation());
		const float MinSafeDistance = 500.f; // 최소 안전 거리

		// 플레이어가 너무 가까이 접근했고, 현재 이동 중이 아니면 다른 엄폐물로 재이동
		if (DistanceToPlayer < MinSafeDistance && !InstanceData.bIsMoving)
		{
			// 새로운 엄폐 위치 찾기
			FVector NewHideLocation = InstanceData.Controller->FindHideLocation(1000.f, Target);
			if (!NewHideLocation.IsZero() && FVector::Dist(NewHideLocation, InstanceData.CurrentHideLocation) > 200.f)
			{
				// 현재 위치와 충분히 떨어진 새로운 위치로 이동
				InstanceData.CurrentHideLocation = NewHideLocation;
				InstanceData.Controller->MoveToLocation(NewHideLocation);
				InstanceData.bIsMoving = true;
			}
		}

		// 뒷걸음질 대상 업데이트: 다른 플레이어가 보고 있으면 갱신
		if (InstanceData.bEnableBackpedal && Stalker)
		{
			if (UWorld* World = Stalker->GetWorld())
			{
				TArray<AActor*> AllPlayers;
				UGameplayStatics::GetAllActorsOfClass(World, AAO_PlayerCharacter::StaticClass(), AllPlayers);

				float ClosestDistance = FLT_MAX;
				AActor* NewLookingPlayer = nullptr;

				for (AActor* PlayerActor : AllPlayers)
				{
					if (InstanceData.Controller->IsPlayerLookingAtMe(PlayerActor))
					{
						const float Distance = FVector::Dist(Stalker->GetActorLocation(), PlayerActor->GetActorLocation());
						if (Distance < ClosestDistance)
						{
							ClosestDistance = Distance;
							NewLookingPlayer = PlayerActor;
						}
					}
				}

				InstanceData.LookingPlayer = NewLookingPlayer;
			}
		}
	}

	// 이동 완료 확인
	if (InstanceData.bIsMoving)
	{
		if (InstanceData.Controller->GetMoveStatus() == EPathFollowingStatus::Idle)
		{
			// 숨기 완료
			InstanceData.bIsMoving = false;
			
			// 후퇴 모드였다면 후퇴 모드 해제 (다시 숨기 준비)
			if (Stalker && Stalker->IsRetreating())
			{
				Stalker->SetRetreatMode(false);
			}
			
			return EStateTreeRunStatus::Succeeded;
		}
	}

	return EStateTreeRunStatus::Running;
}

void FAO_STTask_Stalk_Hide::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	if (InstanceData.Controller)
	{
		InstanceData.Controller->StopMovement();

		// 뒷걸음질 모드 해제: 회전 설정 원복
		if (AAO_Stalker* Stalker = InstanceData.Controller->GetStalker())
		{
			if (UCharacterMovementComponent* MoveComp = Stalker->GetCharacterMovement())
			{
				// 기본값으로 복원 (이동 방향으로 회전)
				MoveComp->bOrientRotationToMovement = true;
			}
			Stalker->bUseControllerRotationYaw = false;
		}
	}
}

