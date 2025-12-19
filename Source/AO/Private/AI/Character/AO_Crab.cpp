// AO_Crab.cpp

#include "AI/Character/AO_Crab.h"
#include "AI/Component/AO_ItemCarryComponent.h"
#include "AI/Subsystem/AO_AISubsystem.h"
#include "AI/Controller/AO_CrabController.h"
#include "Item/AO_MasterItem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"
#include "AO_Log.h"
#include "AI/Component/AO_AIMemoryComponent.h"
#include "Character/AO_PlayerCharacter.h"
#include "Components/CapsuleComponent.h"

AAO_Crab::AAO_Crab()
{
	// 캡슐 크기 설정 (사진 분석 기반)
	GetCapsuleComponent()->SetCapsuleHalfHeight(42.f);
	GetCapsuleComponent()->SetCapsuleRadius(55.f);

	// 아이템 운반 컴포넌트 생성
	ItemCarryComponent = CreateDefaultSubobject<UAO_ItemCarryComponent>(TEXT("ItemCarryComponent"));

	// 기본 이동 속도 설정
	DefaultMovementSpeed = NormalSpeed;
	AlertMovementSpeed = FleeSpeed;

	// AI Controller 클래스 설정
	AIControllerClass = AAO_CrabController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AAO_Crab::BeginPlay()
{
	Super::BeginPlay();

	// 이동 속도 초기화
	UpdateMovementSpeed();

	// 아이템 이벤트 바인딩
	if (ItemCarryComponent)
	{
		ItemCarryComponent->OnItemPickedUp.AddDynamic(this, &AAO_Crab::OnItemPickedUp);
		ItemCarryComponent->OnItemDropped.AddDynamic(this, &AAO_Crab::OnItemDropped);
	}
}

void AAO_Crab::HandleStunBegin()
{
	Super::HandleStunBegin();

	// 기절 시 아이템 드롭 - ItemCarryComponent는 생성자에서 생성되므로 항상 존재해야 함
	if (ensure(ItemCarryComponent) && ItemCarryComponent->IsCarryingItem())
	{
		ItemCarryComponent->ForceDropItem();
		AO_LOG(LogKSJ, Log, TEXT("Crab stunned - dropped item"));
	}

	// 도망 상태 해제
	bIsFleeingFromPlayer = false;
}

void AAO_Crab::HandleStunEnd()
{
	Super::HandleStunEnd();

	// 이동 속도 복구
	UpdateMovementSpeed();
}

void AAO_Crab::SetFleeMode(bool bFleeing)
{
	if (bIsFleeingFromPlayer != bFleeing)
	{
		bIsFleeingFromPlayer = bFleeing;
		UpdateMovementSpeed();

		AO_LOG(LogKSJ, Log, TEXT("Crab Flee Mode: %s"), bFleeing ? TEXT("ON") : TEXT("OFF"));
	}
}

bool AAO_Crab::IsCarryingItem() const
{
	return ItemCarryComponent && ItemCarryComponent->IsCarryingItem();
}

FVector AAO_Crab::CalculateItemDropLocation(const FVector& ExcludeLocation) const
{
	// 아이템을 들고 있지 않으면 현재 위치 반환
	if (!IsCarryingItem())
	{
		return GetActorLocation();
	}

	// AISubsystem을 통해 플레이어들로부터 먼 위치 찾기
	UWorld* World = GetWorld();
	if (!ensure(World))
	{
		return GetActorLocation();
	}

	if (UAO_AISubsystem* Subsystem = World->GetSubsystem<UAO_AISubsystem>())
	{
		// 1. 캐시된 플레이어 위치 (아이템 줍을 때 기록)
		TArray<FVector> PlayerLocations = CachedPlayerLocationsOnPickup;

		// 2. 현재 시야 내 플레이어 위치 (우선순위 높음)
		if (AAO_CrabController* CrabController = Cast<AAO_CrabController>(GetController()))
		{
			TArray<AAO_PlayerCharacter*> PlayersInSight = CrabController->GetPlayersInSight();
			for (AAO_PlayerCharacter* Player : PlayersInSight)
			{
				if (Player)
				{
					// 시야 내 플레이어는 가중치를 높이기 위해 여러 번 추가
					PlayerLocations.Add(Player->GetActorLocation());
					PlayerLocations.Add(Player->GetActorLocation());
				}
			}
		}

		// 3. Memory에 저장된 최근 플레이어 위치 (피해야 할 위치 - 별도 관리)
		TArray<FVector> RecentThreatLocations; // 최근에 플레이어를 본 위치 (회피 우선순위 높음)
		if (UAO_AIMemoryComponent* Memory = GetMemoryComponent())
		{
			// 최근 30초 이내에 플레이어를 본 위치들 (더 강력하게 회피)
			RecentThreatLocations = Memory->GetRecentLostLocations(30.f);
			
			// 소리로 감지된 위치도 위협으로 간주
			FVector HeardLocation = Memory->GetLastHeardLocation();
			if (!HeardLocation.IsZero())
			{
				RecentThreatLocations.Add(HeardLocation);
			}
			
			// 일반 PlayerLocations에도 추가 (거리 계산용)
			for (const FVector& Loc : RecentThreatLocations)
			{
				PlayerLocations.Add(Loc);
			}
		}

		if (PlayerLocations.Num() == 0)
		{
			// 플레이어 위치 정보가 없으면 랜덤 위치
			return Subsystem->FindLocationFarthestFromPlayers(GetActorLocation(), ItemDropSearchRadius);
		}

		// 가장 먼 위치 찾기
		UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
		if (!NavSys)
		{
			return GetActorLocation();
		}

		FVector BestLocation = FVector::ZeroVector; // 기본값을 ZeroVector로 변경 (실패 감지용)
		float BestMinDistance = 0.f;

		// 랜덤 시작 오프셋 추가 (매번 다른 순서로 샘플링하여 다른 결과 유도)
		const int32 NumSamples = 32;
		const int32 RandomOffset = FMath::RandRange(0, NumSamples - 1);
		
		// 제외할 위치와의 최소 거리
		const float ExcludeRadius = 800.f;
		
		for (int32 i = 0; i < NumSamples; ++i)
		{
			// 랜덤 오프셋을 적용하여 매번 다른 순서로 탐색
			const int32 ShuffledIndex = (i + RandomOffset) % NumSamples;
			const float Angle = (2.f * PI * ShuffledIndex) / NumSamples;
			const FVector SampleDir = FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f);
			const FVector SamplePoint = GetActorLocation() + SampleDir * ItemDropSearchRadius;

			FNavLocation NavLocation;
			if (NavSys->ProjectPointToNavigation(SamplePoint, NavLocation))
			{
				// 0차 필터링: 이전에 시도했던 위치(ExcludeLocation) 근처는 제외
				if (!ExcludeLocation.IsZero() && FVector::Dist(NavLocation.Location, ExcludeLocation) < ExcludeRadius)
				{
					continue; // 이전에 실패한 위치 근처는 스킵
				}
				
				// 1차 필터링: 최근 위협 위치(플레이어를 본 곳) 반경 1500 이내는 완전히 제외
				bool bIsTooCloseToRecentThreat = false;
				const float RecentThreatAvoidanceRadius = 1500.f; // 최근 위협에 대한 더 큰 회피 반경
				
				for (const FVector& ThreatLoc : RecentThreatLocations)
				{
					if (FVector::Dist(NavLocation.Location, ThreatLoc) < RecentThreatAvoidanceRadius)
					{
						bIsTooCloseToRecentThreat = true;
						break;
					}
				}
				
				if (bIsTooCloseToRecentThreat)
				{
					continue; // 최근 위협 위치 근처는 아예 고려하지 않음
				}

				// 2차 필터링: 일반 플레이어 위치 반경 1000 이내는 제외
				bool bIsTooCloseToPlayer = false;
				const float MinSafeDistance = 1000.f;

				for (const FVector& PlayerLoc : PlayerLocations)
				{
					if (FVector::Dist(NavLocation.Location, PlayerLoc) < MinSafeDistance)
					{
						bIsTooCloseToPlayer = true;
						break;
					}
				}

				// 위험한 위치면 점수 계산조차 하지 않고 스킵
				if (bIsTooCloseToPlayer)
				{
					continue;
				}

				// 이 위치에서 가장 가까운 플레이어까지의 거리 계산
				float MinDistToPlayer = FLT_MAX;
				for (const FVector& PlayerLoc : PlayerLocations)
				{
					const float Dist = FVector::Dist(NavLocation.Location, PlayerLoc);
					MinDistToPlayer = FMath::Min(MinDistToPlayer, Dist);
				}

				if (MinDistToPlayer > BestMinDistance)
				{
					BestMinDistance = MinDistToPlayer;
					BestLocation = NavLocation.Location;
				}
			}
		}

		// 유효한 위치를 찾았으면 반환, 못찾았으면 현재 위치 반환
		if (!BestLocation.IsZero())
		{
			return BestLocation;
		}
		
		AO_LOG(LogKSJ, Warning, TEXT("Crab: Could not find safe drop location, using current location"));
		return GetActorLocation();
	}

	return GetActorLocation();
}

void AAO_Crab::UpdateMovementSpeed()
{
	// CharacterMovementComponent는 Character의 필수 컴포넌트
	UCharacterMovementComponent* MovementComp = GetCharacterMovement();
	if (!ensure(MovementComp))
	{
		return;
	}

	float NewSpeed = NormalSpeed;

	// 아이템 소지 중이면 무조건 느린 속도
	if (IsCarryingItem())
	{
		NewSpeed = CarryingSpeed;
	}
	// 도망 중이면 빠른 속도 (아이템 미소지 시에만)
	else if (bIsFleeingFromPlayer)
	{
		NewSpeed = FleeSpeed;
	}

	MovementComp->MaxWalkSpeed = NewSpeed;

	AO_LOG(LogKSJ, Verbose, TEXT("Crab Speed Updated: %.1f"), NewSpeed);
}

void AAO_Crab::OnItemPickedUp(AAO_MasterItem* Item)
{
	// 아이템 줍는 순간 모든 플레이어 위치 캐시
	UWorld* World = GetWorld();
	if (ensure(World))
	{
		if (UAO_AISubsystem* Subsystem = World->GetSubsystem<UAO_AISubsystem>())
		{
			CachedPlayerLocationsOnPickup = Subsystem->GetAllPlayerLocations();
		}
	}

	// 속도 업데이트
	UpdateMovementSpeed();

	// Item이 유효한지 확인 후 로그 출력
	if (ensure(Item))
	{
		AO_LOG(LogKSJ, Log, TEXT("Crab picked up item: %s, Cached %d player locations"), 
			*Item->GetName(), CachedPlayerLocationsOnPickup.Num());
	}
}

void AAO_Crab::OnItemDropped(AAO_MasterItem* Item)
{
	// 캐시된 플레이어 위치 초기화
	CachedPlayerLocationsOnPickup.Empty();

	// 속도 업데이트
	UpdateMovementSpeed();

	// Item이 유효한지 확인 후 로그 출력
	if (ensure(Item))
	{
		AO_LOG(LogKSJ, Log, TEXT("Crab dropped item: %s"), *Item->GetName());
	}
}
