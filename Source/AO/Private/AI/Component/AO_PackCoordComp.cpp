//KSJ : AO_PackCoordComp


#include "AI/Component/AO_PackCoordComp.h"
#include "AI/Character/AO_Werewolf.h"
#include "AI/Controller/AO_WerewolfController.h"
#include "Character/AO_PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/OverlapResult.h"
#include "NavigationSystem.h"

UAO_PackCoordComp::UAO_PackCoordComp()
{
	PrimaryComponentTick.bCanEverTick = false; // Tick 불필요
}


void UAO_PackCoordComp::BeginPlay()
{
	Super::BeginPlay();

}


void UAO_PackCoordComp::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

FAO_HowlResult UAO_PackCoordComp::BroadcastHowl(AActor* Target)
{
	FAO_HowlResult Result;
	
	if (!GetOwner())
	{
		return Result;
	}

	// Howl 실행자로 표시
	bIsHowlInitiator = true;

	// 주변의 모든 Werewolf 검색
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner()); // 자신 제외

	// Pawn 채널로 검색 (설정에 따라 변경 가능)
	bool bHit = GetWorld()->OverlapMultiByObjectType(
		Overlaps,
		GetOwner()->GetActorLocation(),
		FQuat::Identity,
		FCollisionObjectQueryParams(ECollisionChannel::ECC_Pawn),
		FCollisionShape::MakeSphere(HowlRadius),
		Params
	);

	int32 AllyCount = 0;

	if (bHit)
	{
		for (const FOverlapResult& OverlapResult : Overlaps)
		{
			AActor* OverlapActor = OverlapResult.GetActor();
			if (OverlapActor && OverlapActor != GetOwner())
			{
				// Werewolf인지 확인
				if (AAO_Werewolf* Wolf = Cast<AAO_Werewolf>(OverlapActor))
				{
					if (UAO_PackCoordComp* AllyComp = Wolf->FindComponentByClass<UAO_PackCoordComp>())
					{
						AllyComp->ReceiveHowl(GetOwner(), Target);
						AllyCount++;
					}
				}
			}
		}
	}

	Result.bFoundAlly = (AllyCount > 0);
	Result.AllyCount = AllyCount;

	// 동료가 있으면 포위 모드 시작
	if (Result.bFoundAlly)
	{
		SetSurroundMode(true);
	}
	else
	{
		// 동료가 없으면 포위 모드 해제 (일반 추격/공격으로)
		SetSurroundMode(false);
	}

	return Result;
}

void UAO_PackCoordComp::ReceiveHowl(AActor* Sender, AActor* Target)
{

	// 이미 포위 중이거나 공격 중이면 무시할 수도 있음 (기획에 따라 다름)
	// 여기서는 Howl을 들으면 무조건 포위 모드로 동기화한다고 가정

	if (!bIsSurrounding)
	{
		SetSurroundMode(true);
		
		// 델리게이트 방송 (StateTree Condition 등에서 반응)
		if (OnHowlReceived.IsBound())
		{
			OnHowlReceived.Broadcast(Target);
		}
	}
}

TArray<AAO_Werewolf*> UAO_PackCoordComp::GetNearbyPackMembers() const
{
	TArray<AAO_Werewolf*> Members;
	if (!GetOwner()) return Members;

	// 전체 검색보다는 Overlap이나 관리되는 리스트가 좋지만, 
	// 여기서는 편의상 OverlapMulti를 다시 사용하거나, 
	// BroadcastHowl 시 캐싱된 정보를 쓸 수도 있음.
	// EQS Context에서 호출될 함수이므로, 실시간으로 주변을 찾는게 안전함.

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());

	GetWorld()->OverlapMultiByObjectType(
		Overlaps,
		GetOwner()->GetActorLocation(),
		FQuat::Identity,
		FCollisionObjectQueryParams(ECollisionChannel::ECC_Pawn),
		FCollisionShape::MakeSphere(HowlRadius), // Howl 반경 내 멤버를 Pack으로 간주
		Params
	);

	for (const FOverlapResult& Result : Overlaps)
	{
		if (AAO_Werewolf* Wolf = Cast<AAO_Werewolf>(Result.GetActor()))
		{
			Members.Add(Wolf);
		}
	}

	return Members;
}

void UAO_PackCoordComp::SetSurroundMode(bool bSurround)
{
	bIsSurrounding = bSurround;
	bIsCoordinatedAttackStarted = false; // 포위 모드 전환 시 리셋

	if (bIsSurrounding)
	{
		// 포위 시작 시 일제 공격 타이머 가동
		float Delay = FMath::RandRange(MinAttackDelay, MaxAttackDelay);
		GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, this, &UAO_PackCoordComp::ExecuteAttack, Delay, false);
		bHasReachedSurroundPosition = false;
		TargetSurroundPosition = FVector::ZeroVector;
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(AttackTimerHandle);
		// 포위 위치 예약 해제
		if (AAO_Werewolf* OwnerWolf = Cast<AAO_Werewolf>(GetOwner()))
		{
			ReleaseSurroundPosition(OwnerWolf);
			ReleaseEscapeRoute(OwnerWolf);
		}
	}
}

void UAO_PackCoordComp::StartCoordinatedAttack()
{
	// 외부(StateTree 등)에서 강제 호출 시
	if (GetWorld()->GetTimerManager().IsTimerActive(AttackTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(AttackTimerHandle);
	}
	ExecuteAttack();
}

void UAO_PackCoordComp::ForceStartCoordinatedAttack()
{
	// 타이머 무시하고 즉시 일제공격 시작
	if (GetWorld()->GetTimerManager().IsTimerActive(AttackTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(AttackTimerHandle);
	}
	ExecuteAttack();
}

void UAO_PackCoordComp::ExecuteAttack()
{
	// 포위 끝, 공격 시작
	bIsSurrounding = false;
	bIsCoordinatedAttackStarted = true;

	// 주변 동료들에게 일제공격 시작 알림
	if (OnCoordinatedAttackStarted.IsBound())
	{
		OnCoordinatedAttackStarted.Broadcast();
	}

	// 주변 Werewolf들에게도 일제공격 알림 전파
	TArray<AAO_Werewolf*> NearbyMembers = GetNearbyPackMembers();
	for (AAO_Werewolf* Member : NearbyMembers)
	{
		if (Member && IsValid(Member))
		{
			if (UAO_PackCoordComp* MemberComp = Member->GetPackCoordComp())
			{
				// 이미 일제공격이 시작되지 않았다면 시작
				if (!MemberComp->IsCoordinatedAttackStarted())
				{
					MemberComp->bIsCoordinatedAttackStarted = true;
					MemberComp->bIsSurrounding = false;
					
					if (MemberComp->OnCoordinatedAttackStarted.IsBound())
					{
						MemberComp->OnCoordinatedAttackStarted.Broadcast();
					}
				}
			}
		}
	}
}

bool UAO_PackCoordComp::TryReserveSurroundPosition(const FVector& Location, AAO_Werewolf* Reserver, float ReservationTimeout)
{
	if (!Reserver || !IsValid(Reserver))
	{
		return false;
	}

	// 이미 예약된 위치인지 확인
	for (const FAO_SurroundPosition& Reserved : ReservedPositions)
	{
		if (Reserved.ReservedBy.IsValid() && Reserved.ReservedBy.Get() != Reserver)
		{
			// 다른 Werewolf가 예약한 위치와 너무 가까우면 실패
			float Distance = FVector::Dist(Location, Reserved.Location);
			if (Distance < 200.f) // 최소 거리
			{
				return false;
			}
		}
	}

	// 만료된 예약 정리
	CleanupExpiredReservations();

	// 기존 예약이 있으면 해제
	ReleaseSurroundPosition(Reserver);

	// 새 위치 예약
	FAO_SurroundPosition NewReservation;
	NewReservation.Location = Location;
	NewReservation.ReservedBy = Reserver;
	NewReservation.ReservationTime = GetWorld()->GetTimeSeconds();
	
	ReservedPositions.Add(NewReservation);
	return true;
}

void UAO_PackCoordComp::ReleaseSurroundPosition(AAO_Werewolf* Reserver)
{
	if (!Reserver)
	{
		return;
	}

	ReservedPositions.RemoveAll([Reserver](const FAO_SurroundPosition& Pos)
	{
		return Pos.ReservedBy.IsValid() && Pos.ReservedBy.Get() == Reserver;
	});
}

FVector UAO_PackCoordComp::FindAvailableSurroundPosition(AAO_PlayerCharacter* Target, float Radius, AAO_Werewolf* ExcludeWolf, float MinDistanceFromOthers)
{
	if (!Target || !GetOwner())
	{
		return FVector::ZeroVector;
	}

	FVector PlayerLocation = Target->GetActorLocation();

	// 만료된 예약 정리
	CleanupExpiredReservations();

	// 플레이어 주변을 원형으로 샘플링하여 사용 가능한 위치 찾기
	FVector BestLocation = FVector::ZeroVector;
	float BestScore = -1.f;

	const int32 NumSamples = 16;
	const float AngleStep = 2.f * UE_PI / NumSamples;

	for (int32 i = 0; i < NumSamples; ++i)
	{
		float Angle = AngleStep * i;
		FVector Direction = FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f);
		FVector SampleLocation = PlayerLocation + Direction * Radius;

		// NavMesh 프로젝션
		UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
		if (NavSys)
		{
			FNavLocation NavLocation;
			if (NavSys->ProjectPointToNavigation(SampleLocation, NavLocation))
			{
				// 다른 Werewolf와의 거리 체크
				float MinDistToOthers = FLT_MAX;
				for (const FAO_SurroundPosition& Reserved : ReservedPositions)
				{
					if (Reserved.ReservedBy.IsValid() && Reserved.ReservedBy.Get() != ExcludeWolf)
					{
						float Dist = FVector::Dist(NavLocation.Location, Reserved.Location);
						MinDistToOthers = FMath::Min(MinDistToOthers, Dist);
					}
				}

				// 최소 거리 이상 떨어져 있고, 플레이어와 적절한 거리
				if (MinDistToOthers >= MinDistanceFromOthers)
				{
					float DistToPlayer = FVector::Dist(NavLocation.Location, PlayerLocation);
					float Score = MinDistToOthers * 0.5f + (Radius - FMath::Abs(DistToPlayer - Radius)) * 0.5f;

					if (Score > BestScore)
					{
						BestScore = Score;
						BestLocation = NavLocation.Location;
					}
				}
			}
		}
	}

	return BestLocation;
}

void UAO_PackCoordComp::CheckSurroundPositionReached(const FVector& CurrentLocation, float AcceptanceRadius)
{
	if (TargetSurroundPosition.IsZero())
	{
		return;
	}

	float Distance = FVector::Dist(CurrentLocation, TargetSurroundPosition);
	if (Distance <= AcceptanceRadius)
	{
		bHasReachedSurroundPosition = true;
	}
}

void UAO_PackCoordComp::CleanupExpiredReservations()
{
	if (!GetWorld())
	{
		return;
	}

	float CurrentTime = GetWorld()->GetTimeSeconds();
	
	ReservedPositions.RemoveAll([CurrentTime, this](const FAO_SurroundPosition& Pos)
	{
		// 예약자가 유효하지 않거나 시간이 만료되었으면 제거
		if (!Pos.ReservedBy.IsValid())
		{
			return true;
		}
		
		if ((CurrentTime - Pos.ReservationTime) > PositionReservationTimeout)
		{
			return true;
		}
		
		return false;
	});
}

int32 UAO_PackCoordComp::AssignEscapeRouteToWolf(AAO_PlayerCharacter* Target, AAO_Werewolf* Wolf, float SearchRadius)
{
	if (!Target || !Wolf || !GetOwner())
	{
		return -1;
	}

	// 주변 Werewolf Controller를 통해 도주 경로 탐색
	AAO_WerewolfController* AnyController = nullptr;
	TArray<AAO_Werewolf*> PackMembers = GetNearbyPackMembers();
	PackMembers.Add(Cast<AAO_Werewolf>(GetOwner()));

	for (AAO_Werewolf* Member : PackMembers)
	{
		if (Member && IsValid(Member))
		{
			if (AAO_WerewolfController* Ctrl = Cast<AAO_WerewolfController>(Member->GetController()))
			{
				AnyController = Ctrl;
				break;
			}
		}
	}

	// 자신의 Controller도 확인
	if (!AnyController)
	{
		if (AAO_Werewolf* OwnerWolf = Cast<AAO_Werewolf>(GetOwner()))
		{
			AnyController = Cast<AAO_WerewolfController>(OwnerWolf->GetController());
		}
	}

	if (!AnyController)
	{
		return -1;
	}

	// 도주 경로 탐색 (개선된 버전)
	TArray<FVector> EscapeRoutes = AnyController->FindPotentialEscapeRoutesImproved(Target, SearchRadius, 8);
	
	if (EscapeRoutes.Num() == 0)
	{
		return -1;
	}

	// 기존 예약 정리
	CleanupExpiredEscapeRouteReservations();

	// 캐시 업데이트 (모든 Werewolf가 같은 도주 경로 목록 공유)
	CachedEscapeRoutes = EscapeRoutes;

	// 사용 가능한 도주로 찾기
	for (int32 i = 0; i < EscapeRoutes.Num(); ++i)
	{
		bool bAlreadyReserved = false;
		for (const FAO_EscapeRouteReservation& Reservation : ReservedEscapeRoutes)
		{
			if (Reservation.ReservedBy.IsValid() && Reservation.RouteIndex == i)
			{
				bAlreadyReserved = true;
				break;
			}
		}

		if (!bAlreadyReserved)
		{
			// 이 Werewolf의 기존 예약 해제
			ReleaseEscapeRoute(Wolf);

			// 새 예약 생성
			FAO_EscapeRouteReservation NewReservation;
			NewReservation.RouteLocation = EscapeRoutes[i];
			NewReservation.ReservedBy = Wolf;
			NewReservation.ReservationTime = GetWorld()->GetTimeSeconds();
			NewReservation.RouteIndex = i;

			ReservedEscapeRoutes.Add(NewReservation);

			return i;
		}
	}

	return -1;  // 모든 도주로가 예약됨
}

FVector UAO_PackCoordComp::GetAssignedEscapeRouteBlockPosition(AAO_Werewolf* Wolf, AAO_PlayerCharacter* Target, float BlockRadius)
{
	if (!Target || !Wolf)
	{
		return FVector::ZeroVector;
	}

	// 이 Werewolf가 예약한 도주로 찾기
	for (const FAO_EscapeRouteReservation& Reservation : ReservedEscapeRoutes)
	{
		if (Reservation.ReservedBy.IsValid() && Reservation.ReservedBy.Get() == Wolf)
		{
			FVector RouteLocation = Reservation.RouteLocation;
			FVector PlayerLocation = Target->GetActorLocation();

			// 플레이어와 도주 경로 사이의 차단 위치 계산
			FVector BlockPosition = PlayerLocation + (RouteLocation - PlayerLocation) * 0.5f;

			// NavMesh 프로젝션
			UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
			if (NavSys)
			{
				FNavLocation NavLocation;
				if (NavSys->ProjectPointToNavigation(BlockPosition, NavLocation))
				{
					return NavLocation.Location;
				}
			}

			return BlockPosition;
		}
	}

	return FVector::ZeroVector;
}

void UAO_PackCoordComp::ReleaseEscapeRoute(AAO_Werewolf* Wolf)
{
	if (!Wolf)
	{
		return;
	}

	ReservedEscapeRoutes.RemoveAll([Wolf](const FAO_EscapeRouteReservation& Reservation)
	{
		return Reservation.ReservedBy.IsValid() && Reservation.ReservedBy.Get() == Wolf;
	});
}

void UAO_PackCoordComp::CleanupExpiredEscapeRouteReservations()
{
	if (!GetWorld())
	{
		return;
	}

	float CurrentTime = GetWorld()->GetTimeSeconds();
	
	ReservedEscapeRoutes.RemoveAll([CurrentTime, this](const FAO_EscapeRouteReservation& Reservation)
	{
		// 예약자가 유효하지 않거나 시간이 만료되었으면 제거
		if (!Reservation.ReservedBy.IsValid())
		{
			return true;
		}
		
		if ((CurrentTime - Reservation.ReservationTime) > EscapeRouteReservationTimeout)
		{
			return true;
		}
		
		return false;
	});
}
