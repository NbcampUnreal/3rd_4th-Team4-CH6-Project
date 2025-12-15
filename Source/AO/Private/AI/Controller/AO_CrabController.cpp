// AO_CrabController.cpp

#include "AI/Controller/AO_CrabController.h"
#include "AI/Character/AO_Crab.h"
#include "AI/Component/AO_AIMemoryComponent.h"
#include "Character/AO_PlayerCharacter.h"
#include "Item/AO_MasterItem.h"
#include "NavigationSystem.h"
#include "AO_Log.h"

AAO_CrabController::AAO_CrabController()
{
	// Crab 특화 Perception 설정
	SightRadius = 1200.f;
	LoseSightRadius = 1500.f;
	PeripheralVisionAngleDegrees = 120.f;
	HearingRange = 1500.f;
}

void AAO_CrabController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	CachedCrab = Cast<AAO_Crab>(InPawn);
}

AAO_Crab* AAO_CrabController::GetCrab() const
{
	return CachedCrab.Get();
}

bool AAO_CrabController::ShouldFlee() const
{
	// 시야 내에 플레이어가 있으면 도망
	if (HasPlayerInSight())
	{
		return true;
	}

	// 근접 거리 내에 플레이어가 있는지 확인
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return false;
	}

	for (const TWeakObjectPtr<AAO_PlayerCharacter>& WeakPlayer : PlayersInSight)
	{
		if (WeakPlayer.IsValid())
		{
			const float Dist = FVector::Dist(ControlledPawn->GetActorLocation(), WeakPlayer->GetActorLocation());
			if (Dist < PlayerProximityThreshold)
			{
				return true;
			}
		}
	}

	return false;
}

FVector AAO_CrabController::GetNearestThreatLocation() const
{
	AAO_PlayerCharacter* NearestPlayer = GetNearestPlayerInSight();
	if (NearestPlayer)
	{
		return NearestPlayer->GetActorLocation();
	}

	// 시야 내 플레이어가 없으면 최근 기억된 위치 확인
	if (AAO_Crab* Crab = GetCrab())
	{
		if (UAO_AIMemoryComponent* Memory = Crab->GetMemoryComponent())
		{
			TArray<FVector> RecentLocations = Memory->GetRecentLostLocations(30.f);
			if (RecentLocations.Num() > 0)
			{
				// 가장 가까운 기억된 위치 반환
				float NearestDistSq = FLT_MAX;
				FVector NearestLoc = FVector::ZeroVector;
				const FVector MyLoc = Crab->GetActorLocation();

				for (const FVector& Loc : RecentLocations)
				{
					const float DistSq = FVector::DistSquared(MyLoc, Loc);
					if (DistSq < NearestDistSq)
					{
						NearestDistSq = DistSq;
						NearestLoc = Loc;
					}
				}

				return NearestLoc;
			}
		}
	}

	return FVector::ZeroVector;
}

FVector AAO_CrabController::CalculateFleeLocation() const
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return FVector::ZeroVector;
	}

	const FVector MyLocation = ControlledPawn->GetActorLocation();
	const FVector ThreatLocation = GetNearestThreatLocation();

	if (ThreatLocation.IsZero())
	{
		return MyLocation;
	}

	// 위협으로부터 반대 방향으로 도망
	FVector FleeDirection = (MyLocation - ThreatLocation).GetSafeNormal();

	// 피해야 할 위치들 (최근 플레이어 목격 위치)
	TArray<FVector> AvoidLocations;
	if (AAO_Crab* Crab = GetCrab())
	{
		if (UAO_AIMemoryComponent* Memory = Crab->GetMemoryComponent())
		{
			AvoidLocations = Memory->GetRecentLostLocations(60.f);
		}
	}

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSys)
	{
		return MyLocation + FleeDirection * FleeSearchRadius;
	}

	FVector BestLocation = MyLocation;
	float BestScore = -FLT_MAX;

	// 여러 방향 샘플링
	const int32 NumSamples = 12;
	for (int32 i = 0; i < NumSamples; ++i)
	{
		// 기본 도망 방향에서 약간씩 변형
		const float AngleOffset = FMath::DegreesToRadians((i - NumSamples / 2) * 30.f);
		FVector SampleDir = FleeDirection.RotateAngleAxis(FMath::RadiansToDegrees(AngleOffset), FVector::UpVector);
		FVector SamplePoint = MyLocation + SampleDir * FleeSearchRadius;

		FNavLocation NavLocation;
		if (NavSys->ProjectPointToNavigation(SamplePoint, NavLocation))
		{
			// 점수 계산: 위협으로부터의 거리 + 피해야 할 위치들로부터의 거리
			float Score = FVector::Dist(NavLocation.Location, ThreatLocation);

			// 피해야 할 위치들과의 거리 보너스
			for (const FVector& AvoidLoc : AvoidLocations)
			{
				Score += FVector::Dist(NavLocation.Location, AvoidLoc) * 0.3f;
			}

			if (Score > BestScore)
			{
				BestScore = Score;
				BestLocation = NavLocation.Location;
			}
		}
	}

	return BestLocation;
}

void AAO_CrabController::OnPlayerDetected(AAO_PlayerCharacter* Player, const FVector& Location)
{
	Super::OnPlayerDetected(Player, Location);

	UpdateFleeState();
}

void AAO_CrabController::OnPlayerLost(AAO_PlayerCharacter* Player, const FVector& LastKnownLocation)
{
	Super::OnPlayerLost(Player, LastKnownLocation);

	UpdateFleeState();
}

void AAO_CrabController::OnNoiseHeard(AActor* NoiseInstigator, const FVector& Location, float Volume)
{
	Super::OnNoiseHeard(NoiseInstigator, Location, Volume);

	// 소리가 들린 위치를 피해야 할 위치로 기억 - Crab은 소리에 반응하여 도망
	AAO_Crab* Crab = GetCrab();
	if (!Crab)
	{
		return;
	}

	UAO_AIMemoryComponent* Memory = Crab->GetMemoryComponent();
	if (!Memory)
	{
		return;
	}

	// 소리 감지 위치 저장
	Memory->SetLastHeardLocation(Location);

	// 플레이어가 소리를 낸 경우 해당 플레이어의 위치도 기억
	AAO_PlayerCharacter* Player = Cast<AAO_PlayerCharacter>(NoiseInstigator);
	if (Player)
	{
		Memory->SetLastKnownLocation(Player, Location);
	}
}

void AAO_CrabController::OnActorDetected(AActor* Actor, const FVector& Location)
{
	Super::OnActorDetected(Actor, Location);

	// Actor가 null일 수 있음 (정상적인 상황)
	if (!Actor)
	{
		return;
	}

	// 아이템인지 확인 - Crab은 아이템을 발견하면 목록에 추가
	AAO_MasterItem* Item = Cast<AAO_MasterItem>(Actor);
	if (Item)
	{
		OnItemDetected(Item, Location);
	}
}

void AAO_CrabController::UpdateFleeState()
{
	if (AAO_Crab* Crab = GetCrab())
	{
		Crab->SetFleeMode(ShouldFlee());
	}
}

void AAO_CrabController::OnItemDetected(AAO_MasterItem* Item, const FVector& Location)
{
	// Item이 유효해야 감지 처리 가능
	if (!ensureMsgf(Item, TEXT("OnItemDetected called with null Item")))
	{
		return;
	}

	// 발견한 아이템 목록에 추가 (중복 방지) - 시야에서 사라져도 기억 유지
	if (!DiscoveredItems.Contains(Item))
	{
		DiscoveredItems.Add(Item);
	}
}

void AAO_CrabController::OnItemLost(AAO_MasterItem* Item)
{
	// 아이템은 시야에서 사라져도 기억 유지 (목록에서 제거하지 않음)
}

TArray<AAO_MasterItem*> AAO_CrabController::GetDiscoveredItems() const
{
	TArray<AAO_MasterItem*> ValidItems;
	for (const TWeakObjectPtr<AAO_MasterItem>& WeakItem : DiscoveredItems)
	{
		if (WeakItem.IsValid())
		{
			ValidItems.Add(WeakItem.Get());
		}
	}
	return ValidItems;
}

bool AAO_CrabController::HasItemInSight() const
{
	for (const TWeakObjectPtr<AAO_MasterItem>& WeakItem : DiscoveredItems)
	{
		if (WeakItem.IsValid())
		{
			return true;
		}
	}
	return false;
}

AAO_MasterItem* AAO_CrabController::GetNearestDiscoveredItem() const
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return nullptr;
	}

	AAO_MasterItem* NearestItem = nullptr;
	float NearestDistSq = FLT_MAX;

	for (const TWeakObjectPtr<AAO_MasterItem>& WeakItem : DiscoveredItems)
	{
		if (WeakItem.IsValid())
		{
			const float DistSq = FVector::DistSquared(ControlledPawn->GetActorLocation(), WeakItem->GetActorLocation());
			if (DistSq < NearestDistSq)
			{
				NearestDistSq = DistSq;
				NearestItem = WeakItem.Get();
			}
		}
	}

	return NearestItem;
}
