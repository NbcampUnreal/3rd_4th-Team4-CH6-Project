//KSJ : AO_StalkerController

#include "AI/Controller/AO_StalkerController.h"
#include "AI/Character/AO_Stalker.h"
#include "AI/Subsystem/AO_AISubsystem.h"
#include "Character/AO_PlayerCharacter.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "NavigationSystem.h"

AAO_StalkerController::AAO_StalkerController()
{
	// Stalker는 처음 발견한 대상을 집요하게 추적
	// KSJ: 요구사항상 다중 플레이어 상황에서 "가까운 대상"으로 목표를 재설정해야 하므로 고정 모드는 끈다.
	bLockOnFirstTarget = false;
}

void AAO_StalkerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

bool AAO_StalkerController::IsRetreating() const
{
	const AAO_Stalker* Stalker = GetStalker();
	return Stalker ? Stalker->IsRetreating() : false;
}

AAO_Stalker* AAO_StalkerController::GetStalker() const
{
	return Cast<AAO_Stalker>(GetPawn());
}

FVector AAO_StalkerController::FindHideLocation(float Radius, AActor* TargetToHideFrom)
{
	// EQS를 사용하여 엄폐물 찾기
	// 다른 Stalker의 위치를 고려하여 겹치지 않는 위치 선택
	
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return FVector::ZeroVector;
	}

	AAO_Stalker* Stalker = GetStalker();
	if (!Stalker)
	{
		return FVector::ZeroVector;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return FVector::ZeroVector;
	}

	// 다른 Stalker들의 위치 가져오기 (겹침 방지용)
	TArray<FVector> OtherStalkerLocations;
	if (UAO_AISubsystem* Subsystem = World->GetSubsystem<UAO_AISubsystem>())
	{
		OtherStalkerLocations = Subsystem->GetAllStalkerLocations(Stalker);
	}

		// 타겟 위치 (플레이어 또는 마지막으로 본 위치)
	FVector TargetLocation = FVector::ZeroVector;
	if (TargetToHideFrom)
	{
		TargetLocation = TargetToHideFrom->GetActorLocation();
	}
	else
	{
		TargetLocation = GetLastKnownTargetLocation();
		if (TargetLocation.IsZero())
		{
			AAO_PlayerCharacter* CurrentChaseTarget = GetChaseTarget();
			if (CurrentChaseTarget)
			{
				TargetLocation = CurrentChaseTarget->GetActorLocation();
			}
		}
	}

	if (TargetLocation.IsZero())
	{
		return ControlledPawn->GetActorLocation();
	}

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(World);
	if (!NavSys)
	{
		return ControlledPawn->GetActorLocation();
	}

	// 여러 방향으로 샘플링하여 플레이어 시야에서 숨을 수 있는 위치 찾기
	FVector BestLocation = ControlledPawn->GetActorLocation();
	float BestScore = -1.f;

	const int32 NumSamples = 32;
	const float MinDistance = 300.f; // 최소 거리
	const float MaxDistance = Radius;

	for (int32 i = 0; i < NumSamples; ++i)
	{
		const float Angle = (2.f * UE_PI * i) / NumSamples;
		const float Distance = FMath::RandRange(MinDistance, MaxDistance);
		const FVector SampleDir = FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f);
		const FVector SamplePoint = TargetLocation + SampleDir * Distance;

		FNavLocation NavLocation;
		if (NavSys->ProjectPointToNavigation(SamplePoint, NavLocation))
		{
			// 플레이어 시야에서 숨을 수 있는지 체크 (LineTrace)
			FHitResult Hit;
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(ControlledPawn);
			if (TargetToHideFrom)
			{
				Params.AddIgnoredActor(TargetToHideFrom);
			}

			// 타겟에서 엄폐 위치로의 LineTrace
			bool bIsHidden = World->LineTraceSingleByChannel(
				Hit, 
				TargetLocation, 
				NavLocation.Location, 
				ECC_Visibility, 
				Params
			);

			if (bIsHidden)
			{
				// 다른 Stalker와의 거리 체크 (겹침 방지)
				float MinDistToOtherStalker = FLT_MAX;
				for (const FVector& OtherLoc : OtherStalkerLocations)
				{
					const float Dist = FVector::Dist(NavLocation.Location, OtherLoc);
					MinDistToOtherStalker = FMath::Min(MinDistToOtherStalker, Dist);
				}

				// 점수 계산: 엄폐물이 있고, 다른 Stalker와 충분히 떨어져 있고, 적절한 거리
				const float DistToTarget = FVector::Dist(NavLocation.Location, TargetLocation);
				const float StalkerSeparationScore = FMath::Min(MinDistToOtherStalker / 500.f, 1.f); // 500 이상이면 1.0
				const float DistanceScore = 1.f - FMath::Abs(DistToTarget - (MinDistance + MaxDistance) * 0.5f) / MaxDistance;
				
				const float Score = StalkerSeparationScore * 0.5f + DistanceScore * 0.5f;

				if (Score > BestScore && MinDistToOtherStalker > 200.f) // 최소 200 유닛 이상 떨어져야 함
				{
					BestScore = Score;
					BestLocation = NavLocation.Location;
				}
			}
		}
	}

	return BestLocation;
}

void AAO_StalkerController::RequestHideLocationEQS(UEnvQuery* Query)
{
	// KSJ: Task에서 반복 요청할 수 있으므로 in-flight이면 무시한다.
	if (!Query || bHideQueryInFlight)
	{
		return;
	}

	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	bHideQueryInFlight = true;
	const uint32 RequestSerial = ++HideQuerySerial;

	// 결과는 Controller에 저장 후 Task가 Tick에서 Consume한다.
	FEnvQueryRequest Request(Query, ControlledPawn);
	Request.Execute(
		EEnvQueryRunMode::SingleResult,
		FQueryFinishedSignature::CreateWeakLambda(
			this,
			[this, RequestSerial](TSharedPtr<FEnvQueryResult> Result)
			{
				// Controller가 파괴/언포제스 되었거나, 더 최신 요청이 있으면 무시한다.
				bHideQueryInFlight = false;
				if (RequestSerial != HideQuerySerial)
				{
					return;
				}

				if (Result.IsValid() && Result->IsSuccessful())
				{
					PendingHideLocation = Result->GetItemAsLocation(0);
					bHasPendingHideLocation = !PendingHideLocation.IsZero();
				}
			}
		)
	);
}

bool AAO_StalkerController::ConsumePendingHideLocation(FVector& OutLocation)
{
	if (!bHasPendingHideLocation)
	{
		return false;
	}

	OutLocation = PendingHideLocation;
	PendingHideLocation = FVector::ZeroVector;
	bHasPendingHideLocation = false;
	return !OutLocation.IsZero();
}

FVector AAO_StalkerController::FindRetreatLocation()
{
	// 플레이어로부터 멀어지고 시야가 가려지는 곳으로 후퇴
	
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return FVector::ZeroVector;
	}

	AAO_PlayerCharacter* Target = GetChaseTarget();
	if (!Target)
	{
		return ControlledPawn->GetActorLocation();
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return ControlledPawn->GetActorLocation();
	}

	FVector PlayerLocation = Target->GetActorLocation();
	FVector StalkerLocation = ControlledPawn->GetActorLocation();
	FVector DirFromPlayer = (StalkerLocation - PlayerLocation).GetSafeNormal();

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(World);
	if (!NavSys)
	{
		return ControlledPawn->GetActorLocation();
	}

	// 플레이어로부터 멀어지는 방향으로 후퇴 위치 찾기
	const float RetreatDistance = 800.f; // 후퇴 거리
	FVector RetreatPoint = StalkerLocation + DirFromPlayer * RetreatDistance;

	FNavLocation NavLocation;
	if (NavSys->ProjectPointToNavigation(RetreatPoint, NavLocation))
	{
		// 플레이어 시야에서 숨을 수 있는지 체크
		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(ControlledPawn);
		Params.AddIgnoredActor(Target);

		bool bIsHidden = World->LineTraceSingleByChannel(
			Hit,
			PlayerLocation,
			NavLocation.Location,
			ECC_Visibility,
			Params
		);

		if (bIsHidden)
		{
			return NavLocation.Location;
		}
	}

	// 숨을 수 없는 경우라도 플레이어로부터 멀어지는 위치 반환
	return NavLocation.Location;
}

void AAO_StalkerController::OnAttackFinished()
{
	// KSJ: Retreat 상태는 Pawn(AAO_Stalker)이 단일 소스로 관리한다.
	if (AAO_Stalker* Stalker = GetStalker())
	{
		Stalker->SetRetreatMode(true);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(RetreatTimerHandle, this, &AAO_StalkerController::OnRetreatTimerExpired, RetreatDuration, false);
	}
}

void AAO_StalkerController::OnRetreatTimerExpired()
{
	// KSJ: Retreat 종료
	if (AAO_Stalker* Stalker = GetStalker())
	{
		Stalker->SetRetreatMode(false);
	}
}

void AAO_StalkerController::OnPlayerDetected(AAO_PlayerCharacter* Player, const FVector& Location)
{
	// 기본 감지/추격 로직은 부모에서 처리 (타겟 설정, LastKnown 갱신 등)
	Super::OnPlayerDetected(Player, Location);

	// KSJ: 혹시라도 Search 타이머가 걸려있으면(과거 로직/에디터 설정 등) 스토킹에 방해되므로 정리한다.
	if (SearchTimerHandle.IsValid())
	{
		GetWorldTimerManager().ClearTimer(SearchTimerHandle);
	}

	// KSJ: 타겟을 다시 확보했으므로 '타겟 유지' 타이머는 해제한다.
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TargetPersistenceTimerHandle);
	}
}

void AAO_StalkerController::OnPlayerLost(AAO_PlayerCharacter* Player, const FVector& LastKnownLocation)
{
	// KSJ:
	// Aggressive 공통 로직은 타겟을 잃으면 즉시 Search로 전환한다.
	// 하지만 Stalker는 "엄폐 접근(스토킹)" 특성상 타겟이 시야에서 사라지는 것이 정상이며,
	// 즉시 Search/배회로 넘어가면 요구사항을 만족할 수 없다.
	//
	// 따라서:
	// - LastKnown 위치는 기록한다 (Super의 Memory 기록은 유지)
	// - Search 모드로 강제 전환하지 않는다
	// - 일정 시간 동안은 타겟을 유지하며 스토킹 루프를 지속한다 (StateTree가 Hide/Approach를 수행)

	AAO_PlayerCharacter* Current = GetChaseTarget();
	const bool bIsCurrentTarget = (Current && Player && Current == Player);

	// 기본 메모리/LastLost 기록은 유지하기 위해 "AIControllerBase" 레벨 처리는 한 번 실행한다.
	// (AAO_AggressiveAICtrl::Super 호출은 Search로 넘어가므로 피한다)
	AAO_AIControllerBase::OnPlayerLost(Player, LastKnownLocation);

	if (bIsCurrentTarget)
	{
		// LastKnown은 AggressiveCtrl이 가진 값을 그대로 유지/갱신한다.
		LastKnownTargetLocation = LastKnownLocation;

		// 타겟 유지 타이머 시작: 시간이 지나도 다시 감지하지 못하면 타겟을 해제한다.
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(TargetPersistenceTimerHandle);
			World->GetTimerManager().SetTimer(
				TargetPersistenceTimerHandle,
				this,
				&AAO_StalkerController::OnTargetPersistenceExpired,
				TargetPersistenceSeconds,
				false
			);
		}
	}
}

void AAO_StalkerController::OnTargetPersistenceExpired()
{
	// KSJ:
	// 타이머 만료 시점에도 시야에 플레이어가 없으면 스토킹 타겟을 해제하고 배회로 복귀 가능하게 한다.
	// (StateTree 상위 전이가 PlayerNearby=false 등을 통해 Roam으로 넘어가도록)
	if (HasPlayerInSight())
	{
		return;
	}

	SetChaseTarget(nullptr);

	if (AAO_AggressiveAIBase* AI = GetAggressiveAI())
	{
		AI->SetChaseMode(false);
		AI->SetSearchMode(false);
	}

	// KSJ: 혹시 Search 타이머가 걸려있으면 정리한다.
	if (SearchTimerHandle.IsValid())
	{
		GetWorldTimerManager().ClearTimer(SearchTimerHandle);
	}

	LastKnownTargetLocation = FVector::ZeroVector;
}

bool AAO_StalkerController::IsPlayerLookingAtMe(AActor* TargetActor, float ToleranceDegrees) const
{
	APawn* MyPawn = GetPawn();
	if (!MyPawn || !TargetActor)
	{
		return false;
	}

	FVector ToStalker = (MyPawn->GetActorLocation() - TargetActor->GetActorLocation()).GetSafeNormal();
	FVector TargetForward = TargetActor->GetActorForwardVector();

	// 내적 계산
	float Dot = FVector::DotProduct(TargetForward, ToStalker);
	float Threshold = FMath::Cos(FMath::DegreesToRadians(ToleranceDegrees));

	// 내적값이 임계값보다 크면 보고 있는 것
	return Dot > Threshold;
}
