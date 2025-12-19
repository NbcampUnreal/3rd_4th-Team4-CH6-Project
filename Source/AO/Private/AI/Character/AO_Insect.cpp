// AO_Insect.cpp

#include "AI/Character/AO_Insect.h"
#include "AI/Component/AO_KidnapComponent.h"
#include "AI/Subsystem/AO_AISubsystem.h"
#include "AI/Controller/AO_InsectController.h"
#include "Character/AO_PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"
#include "AO_Log.h"

AAO_Insect::AAO_Insect()
{
	// 납치 컴포넌트 생성
	KidnapComponent = CreateDefaultSubobject<UAO_KidnapComponent>(TEXT("KidnapComponent"));

	// 기본 속도 설정
	DefaultMovementSpeed = NormalSpeed;
	AlertMovementSpeed = NormalSpeed; // Insect는 추격 속도도 기본과 동일

	// AI Controller 클래스 설정
	AIControllerClass = AAO_InsectController::StaticClass();
}

void AAO_Insect::BeginPlay()
{
	Super::BeginPlay();

	UpdateMovementSpeed();

	if (KidnapComponent)
	{
		KidnapComponent->OnKidnapStateChanged.AddDynamic(this, &AAO_Insect::OnKidnapStateChanged);
	}
}

bool AAO_Insect::IsKidnapping() const
{
	return KidnapComponent && KidnapComponent->IsKidnapping();
}

void AAO_Insect::UpdateMovementSpeed()
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp) return;

	if (IsKidnapping())
	{
		MoveComp->MaxWalkSpeed = DragSpeed;
		
		// 뒷걸음질을 위해 회전 방향 고정 해제 (Controller가 Focus를 잡거나 StateTree Task가 처리)
		MoveComp->bOrientRotationToMovement = false; 
		MoveComp->bUseControllerDesiredRotation = true; // 컨트롤러가 보는 방향(타겟)을 바라봄
	}
	else
	{
		MoveComp->MaxWalkSpeed = NormalSpeed;
		
		// 일반 이동 복구
		MoveComp->bOrientRotationToMovement = true;
		MoveComp->bUseControllerDesiredRotation = false;
	}
}

void AAO_Insect::HandleStunBegin()
{
	Super::HandleStunBegin();

	// 기절 시 납치 해제 (던지지 않고 드롭)
	if (IsKidnapping())
	{
		KidnapComponent->ReleaseKidnap(false);
		AO_LOG(LogKSJ, Log, TEXT("Insect Stunned: Dropped Victim"));
	}
}

void AAO_Insect::HandleStunEnd()
{
	Super::HandleStunEnd();
	UpdateMovementSpeed();
}

void AAO_Insect::OnKidnapStateChanged(bool bIsKidnapping)
{
	UpdateMovementSpeed();

	if (bIsKidnapping)
	{
		// 납치 시작 시 모든 플레이어 위치 캐싱 (AISubsystem 활용)
		if (UWorld* World = GetWorld())
		{
			if (UAO_AISubsystem* Subsystem = World->GetSubsystem<UAO_AISubsystem>())
			{
				CachedPlayerLocations = Subsystem->GetAllPlayerLocations();
				AO_LOG(LogKSJ, Log, TEXT("Insect: Cached %d player locations for kidnap routing"), CachedPlayerLocations.Num());
			}
		}
	}
	else
	{
		CachedPlayerLocations.Empty();
	}
}

FVector AAO_Insect::CalculateSafeDropLocation(const FVector& ExcludeLocation) const
{
	// Crab의 로직과 유사하지만, "모든 플레이어"로부터 멀어져야 함.
	// EQS를 사용하지 않고 C++로 직접 계산하는 방식 (요청사항 반영: "Crab과 비슷하게")
	
	UWorld* World = GetWorld();
	if (!World) return GetActorLocation();

	// 회피 대상 목록 구성
	TArray<FVector> AvoidanceLocations = CachedPlayerLocations;

	// 추가: 현재 시야에 들어온 플레이어 (즉각 반응용)
	if (AAO_InsectController* Ctrl = Cast<AAO_InsectController>(GetController()))
	{
		TArray<AAO_PlayerCharacter*> VisiblePlayers = Ctrl->GetPlayersInSight();
		for (AAO_PlayerCharacter* P : VisiblePlayers)
		{
			if (P && P != KidnapComponent->GetCurrentVictim()) // 납치 대상은 제외 (당연히 가까움)
			{
				AvoidanceLocations.Add(P->GetActorLocation());
				// 시야 내 플레이어는 가중치 2배 (한번 더 추가)
				AvoidanceLocations.Add(P->GetActorLocation());
			}
		}
	}

	if (AvoidanceLocations.Num() == 0)
	{
		return GetActorLocation(); // 피할 대상이 없으면 제자리(혹은 랜덤)
	}

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(World);
	if (!NavSys) return GetActorLocation();

	FVector BestLocation = GetActorLocation();
	float BestScore = -1.f;
	
	const int32 NumSamples = 16;
	const float Radius = SafeLocationSearchRadius;

	for (int32 i = 0; i < NumSamples; ++i)
	{
		// 랜덤 샘플링
		FNavLocation NavLoc;
		if (NavSys->GetRandomReachablePointInRadius(GetActorLocation(), Radius, NavLoc))
		{
			float Score = 0.f;
			float MinDist = FLT_MAX;

			// 모든 회피 대상과의 거리 합산 (또는 최소 거리 최대화)
			for (const FVector& AvoidLoc : AvoidanceLocations)
			{
				float Dist = FVector::Dist(NavLoc.Location, AvoidLoc);
				MinDist = FMath::Min(MinDist, Dist);
			}

			// 최소 거리가 가장 큰 곳을 선택 (Maximin Strategy)
			Score = MinDist;

			if (Score > BestScore)
			{
				BestScore = Score;
				BestLocation = NavLoc.Location;
			}
		}
	}
	
	return BestLocation;
}

