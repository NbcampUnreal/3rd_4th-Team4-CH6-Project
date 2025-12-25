// AO_StalkerController.cpp

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
	bLockOnFirstTarget = true;
}

void AAO_StalkerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
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

