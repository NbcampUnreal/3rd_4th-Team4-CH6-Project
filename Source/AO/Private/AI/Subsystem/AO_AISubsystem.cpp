// AO_AISubsystem.cpp

#include "AI/Subsystem/AO_AISubsystem.h"
#include "Item/AO_MasterItem.h"
#include "Character/AO_PlayerCharacter.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "AO_Log.h"

void UAO_AISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 주기적으로 만료된 예약 정리
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			CleanupTimerHandle,
			FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				CleanupExpiredReservations();
				CleanupExpiredDropCooldowns();
				CleanupExpiredKidnapCooldowns();
			}),
			5.f,
			true
		);
	}

	AO_LOG(LogKSJ, Log, TEXT("AISubsystem Initialized"));
}

void UAO_AISubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CleanupTimerHandle);
	}

	ItemReservations.Empty();
	RecentlyDroppedItems.Empty();
	KidnappedPlayers.Empty();
	RecentlyKidnappedPlayers.Empty();

	Super::Deinitialize();
}

bool UAO_AISubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	// 게임 월드에서만 생성
	if (UWorld* World = Cast<UWorld>(Outer))
	{
		return World->IsGameWorld();
	}
	return false;
}

bool UAO_AISubsystem::TryReserveItem(AAO_MasterItem* Item, AActor* Reserver)
{
	// Item과 Reserver가 유효해야 예약 가능
	if (!ensureMsgf(Item, TEXT("TryReserveItem called with null Item")))
	{
		return false;
	}

	if (!ensureMsgf(Reserver, TEXT("TryReserveItem called with null Reserver")))
	{
		return false;
	}

	// 이미 예약되어 있는지 확인 - 다른 AI가 예약한 경우 실패
	FAO_ItemReservation* Existing = ItemReservations.Find(Item);
	if (Existing)
	{
		if (Existing->ReservedBy.IsValid() && Existing->ReservedBy.Get() != Reserver)
		{
			return false;
		}
	}

	UWorld* World = GetWorld();
	if (!ensure(World))
	{
		return false;
	}

	FAO_ItemReservation Reservation;
	Reservation.ReservedBy = Reserver;
	Reservation.ReservationTime = World->GetTimeSeconds();

	ItemReservations.Add(Item, Reservation);

	AO_LOG(LogKSJ, Log, TEXT("Item Reserved: %s by %s"), *Item->GetName(), *Reserver->GetName());
	return true;
}

void UAO_AISubsystem::ReleaseItem(AAO_MasterItem* Item)
{
	// Item이 유효해야 예약 해제 가능
	if (!ensureMsgf(Item, TEXT("ReleaseItem called with null Item")))
	{
		return;
	}

	ItemReservations.Remove(Item);
	AO_LOG(LogKSJ, Log, TEXT("Item Released: %s"), *Item->GetName());
}

bool UAO_AISubsystem::IsItemReserved(AAO_MasterItem* Item) const
{
	// Item이 null이면 예약되지 않은 것으로 처리
	if (!ensureMsgf(Item, TEXT("IsItemReserved called with null Item")))
	{
		return false;
	}

	const FAO_ItemReservation* Reservation = ItemReservations.Find(Item);
	return Reservation && Reservation->ReservedBy.IsValid();
}

AActor* UAO_AISubsystem::GetItemReserver(AAO_MasterItem* Item) const
{
	// Item이 null이면 예약자 없음
	if (!ensureMsgf(Item, TEXT("GetItemReserver called with null Item")))
	{
		return nullptr;
	}

	const FAO_ItemReservation* Reservation = ItemReservations.Find(Item);
	if (Reservation && Reservation->ReservedBy.IsValid())
	{
		return Reservation->ReservedBy.Get();
	}
	return nullptr;
}

void UAO_AISubsystem::MarkItemAsRecentlyDropped(AAO_MasterItem* Item, float CooldownDuration)
{
	// Item이 유효해야 드롭 기록 가능
	if (!ensureMsgf(Item, TEXT("MarkItemAsRecentlyDropped called with null Item")))
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!ensure(World))
	{
		return;
	}

	FAO_RecentlyDroppedItem DropInfo;
	DropInfo.DropTime = World->GetTimeSeconds();
	DropInfo.CooldownDuration = CooldownDuration;

	RecentlyDroppedItems.Add(Item, DropInfo);

	AO_LOG(LogKSJ, Log, TEXT("Item marked as recently dropped: %s (Cooldown: %.1fs)"), 
		*Item->GetName(), CooldownDuration);
}

bool UAO_AISubsystem::IsItemRecentlyDropped(AAO_MasterItem* Item) const
{
	// Item이 null이면 최근 드롭되지 않은 것으로 처리
	if (!ensureMsgf(Item, TEXT("IsItemRecentlyDropped called with null Item")))
	{
		return false;
	}

	const FAO_RecentlyDroppedItem* DropInfo = RecentlyDroppedItems.Find(Item);
	if (!DropInfo)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!ensure(World))
	{
		return false;
	}

	const float CurrentTime = World->GetTimeSeconds();
	return (CurrentTime - DropInfo->DropTime) < DropInfo->CooldownDuration;
}

bool UAO_AISubsystem::TryReservePlayerForKidnap(AAO_PlayerCharacter* Player, AActor* Kidnapper)
{
	// Player와 Kidnapper가 유효해야 납치 예약 가능
	if (!ensureMsgf(Player, TEXT("TryReservePlayerForKidnap called with null Player")))
	{
		return false;
	}

	if (!ensureMsgf(Kidnapper, TEXT("TryReservePlayerForKidnap called with null Kidnapper")))
	{
		return false;
	}

	// 이미 납치 중인지 확인 - 다른 AI가 납치 중이면 실패
	if (IsPlayerBeingKidnapped(Player))
	{
		return false;
	}

	// 최근 납치된 플레이어인지 확인 - 쿨다운 중이면 실패
	if (IsPlayerRecentlyKidnapped(Player))
	{
		return false;
	}

	KidnappedPlayers.Add(Player, Kidnapper);

	AO_LOG(LogKSJ, Log, TEXT("Player Reserved for Kidnap: %s by %s"), 
		*Player->GetName(), *Kidnapper->GetName());
	return true;
}

void UAO_AISubsystem::ReleasePlayerFromKidnap(AAO_PlayerCharacter* Player)
{
	// Player가 유효해야 납치 해제 가능
	if (!ensureMsgf(Player, TEXT("ReleasePlayerFromKidnap called with null Player")))
	{
		return;
	}

	KidnappedPlayers.Remove(Player);
	AO_LOG(LogKSJ, Log, TEXT("Player Released from Kidnap: %s"), *Player->GetName());
}

bool UAO_AISubsystem::IsPlayerBeingKidnapped(AAO_PlayerCharacter* Player) const
{
	// Player가 null이면 납치 중이 아닌 것으로 처리
	if (!ensureMsgf(Player, TEXT("IsPlayerBeingKidnapped called with null Player")))
	{
		return false;
	}

	const TWeakObjectPtr<AActor>* Kidnapper = KidnappedPlayers.Find(Player);
	return Kidnapper && Kidnapper->IsValid();
}

void UAO_AISubsystem::MarkPlayerAsRecentlyKidnapped(AAO_PlayerCharacter* Player, float CooldownDuration)
{
	// Player가 유효해야 납치 기록 가능
	if (!ensureMsgf(Player, TEXT("MarkPlayerAsRecentlyKidnapped called with null Player")))
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!ensure(World))
	{
		return;
	}

	FAO_RecentlyDroppedItem KidnapInfo;
	KidnapInfo.DropTime = World->GetTimeSeconds();
	KidnapInfo.CooldownDuration = CooldownDuration;

	RecentlyKidnappedPlayers.Add(Player, KidnapInfo);
}

bool UAO_AISubsystem::IsPlayerRecentlyKidnapped(AAO_PlayerCharacter* Player) const
{
	// Player가 null이면 최근 납치되지 않은 것으로 처리
	if (!ensureMsgf(Player, TEXT("IsPlayerRecentlyKidnapped called with null Player")))
	{
		return false;
	}

	const FAO_RecentlyDroppedItem* KidnapInfo = RecentlyKidnappedPlayers.Find(Player);
	if (!KidnapInfo)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!ensure(World))
	{
		return false;
	}

	const float CurrentTime = World->GetTimeSeconds();
	return (CurrentTime - KidnapInfo->DropTime) < KidnapInfo->CooldownDuration;
}

TArray<FVector> UAO_AISubsystem::GetAllPlayerLocations() const
{
	TArray<FVector> Locations;

	UWorld* World = GetWorld();
	if (!ensure(World))
	{
		return Locations;
	}

	// 모든 플레이어 캐릭터의 위치 수집
	// Note: 자주 호출되는 경우 플레이어 목록을 캐싱하는 것이 좋음
	TArray<AActor*> PlayerActors;
	UGameplayStatics::GetAllActorsOfClass(World, AAO_PlayerCharacter::StaticClass(), PlayerActors);

	Locations.Reserve(PlayerActors.Num());
	for (AActor* Actor : PlayerActors)
	{
		if (Actor)
		{
			Locations.Add(Actor->GetActorLocation());
		}
	}

	return Locations;
}

FVector UAO_AISubsystem::FindLocationFarthestFromPlayers(const FVector& Origin, float SearchRadius) const
{
	TArray<FVector> PlayerLocations = GetAllPlayerLocations();
	if (PlayerLocations.Num() == 0)
	{
		return Origin;
	}

	UWorld* World = GetWorld();
	if (!ensure(World))
	{
		return Origin;
	}

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(World);
	if (!NavSys)
	{
		return Origin;
	}

	FVector BestLocation = Origin;
	float BestMinDistance = 0.f;

	// 여러 방향으로 샘플링하여 가장 먼 위치 찾기
	const int32 NumSamples = 16;
	for (int32 i = 0; i < NumSamples; ++i)
	{
		const float Angle = (2.f * PI * i) / NumSamples;
		const FVector SampleDir = FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f);
		const FVector SamplePoint = Origin + SampleDir * SearchRadius;

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

			// 가장 가까운 플레이어와의 거리가 가장 먼 위치 선택
			if (MinDistToPlayer > BestMinDistance)
			{
				BestMinDistance = MinDistToPlayer;
				BestLocation = NavLocation.Location;
			}
		}
	}

	return BestLocation;
}

void UAO_AISubsystem::CleanupExpiredReservations()
{
	const float CurrentTime = GetWorld()->GetTimeSeconds();

	TArray<TWeakObjectPtr<AAO_MasterItem>> ToRemove;
	for (const auto& Pair : ItemReservations)
	{
		if (!Pair.Key.IsValid() || !Pair.Value.ReservedBy.IsValid())
		{
			ToRemove.Add(Pair.Key);
		}
		else if ((CurrentTime - Pair.Value.ReservationTime) > ReservationTimeout)
		{
			ToRemove.Add(Pair.Key);
		}
	}

	for (const auto& Key : ToRemove)
	{
		ItemReservations.Remove(Key);
	}
}

void UAO_AISubsystem::CleanupExpiredDropCooldowns()
{
	const float CurrentTime = GetWorld()->GetTimeSeconds();

	TArray<TWeakObjectPtr<AAO_MasterItem>> ToRemove;
	for (const auto& Pair : RecentlyDroppedItems)
	{
		if (!Pair.Key.IsValid())
		{
			ToRemove.Add(Pair.Key);
		}
		else if ((CurrentTime - Pair.Value.DropTime) > Pair.Value.CooldownDuration)
		{
			ToRemove.Add(Pair.Key);
		}
	}

	for (const auto& Key : ToRemove)
	{
		RecentlyDroppedItems.Remove(Key);
	}
}

void UAO_AISubsystem::CleanupExpiredKidnapCooldowns()
{
	const float CurrentTime = GetWorld()->GetTimeSeconds();

	// 납치 중인 플레이어 정리 (유효하지 않은 참조)
	TArray<TWeakObjectPtr<AAO_PlayerCharacter>> KidnapToRemove;
	for (const auto& Pair : KidnappedPlayers)
	{
		if (!Pair.Key.IsValid() || !Pair.Value.IsValid())
		{
			KidnapToRemove.Add(Pair.Key);
		}
	}
	for (const auto& Key : KidnapToRemove)
	{
		KidnappedPlayers.Remove(Key);
	}

	// 최근 납치 쿨다운 정리
	TArray<TWeakObjectPtr<AAO_PlayerCharacter>> CooldownToRemove;
	for (const auto& Pair : RecentlyKidnappedPlayers)
	{
		if (!Pair.Key.IsValid())
		{
			CooldownToRemove.Add(Pair.Key);
		}
		else if ((CurrentTime - Pair.Value.DropTime) > Pair.Value.CooldownDuration)
		{
			CooldownToRemove.Add(Pair.Key);
		}
	}
	for (const auto& Key : CooldownToRemove)
	{
		RecentlyKidnappedPlayers.Remove(Key);
	}
}
