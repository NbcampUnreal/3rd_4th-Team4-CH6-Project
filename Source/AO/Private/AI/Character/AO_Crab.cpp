// AO_Crab.cpp

#include "AI/Character/AO_Crab.h"
#include "AI/Component/AO_ItemCarryComponent.h"
#include "AI/Subsystem/AO_AISubsystem.h"
#include "AI/Controller/AO_CrabController.h"
#include "Item/AO_MasterItem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"
#include "AO_Log.h"
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

FVector AAO_Crab::CalculateItemDropLocation() const
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
		// 캐시된 플레이어 위치 사용 (아이템 줍을 때 기록)
		// 추가로 현재 시야 내 플레이어 위치도 고려
		TArray<FVector> PlayerLocations = CachedPlayerLocationsOnPickup;

		// 현재 시야 내 플레이어 위치 추가 (우선순위 높음)
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

		FVector BestLocation = GetActorLocation();
		float BestMinDistance = 0.f;

		const int32 NumSamples = 16;
		for (int32 i = 0; i < NumSamples; ++i)
		{
			const float Angle = (2.f * PI * i) / NumSamples;
			const FVector SampleDir = FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f);
			const FVector SamplePoint = GetActorLocation() + SampleDir * ItemDropSearchRadius;

			FNavLocation NavLocation;
			if (NavSys->ProjectPointToNavigation(SamplePoint, NavLocation))
			{
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

		return BestLocation;
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
