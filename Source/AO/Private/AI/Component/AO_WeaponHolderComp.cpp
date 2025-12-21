// AO_WeaponHolderComp.cpp

#include "AI/Component/AO_WeaponHolderComp.h"
#include "AI/Item/AO_TrollWeapon.h"
#include "AI/Character/AO_Troll.h"
#include "Kismet/GameplayStatics.h"
#include "AO_Log.h"

UAO_WeaponHolderComp::UAO_WeaponHolderComp()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAO_WeaponHolderComp::BeginPlay()
{
	Super::BeginPlay();
}

bool UAO_WeaponHolderComp::PickupWeapon(AAO_TrollWeapon* Weapon)
{
	if (!Weapon)
	{
		AO_LOG(LogKSJ, Warning, TEXT("PickupWeapon: Weapon is null"));
		return false;
	}

	// 이미 무기를 들고 있는 경우
	if (CurrentWeapon.IsValid())
	{
		AO_LOG(LogKSJ, Warning, TEXT("PickupWeapon: Already holding a weapon"));
		return false;
	}

	// 이미 다른 Troll이 소지 중인 경우
	if (Weapon->IsPickedUp())
	{
		AO_LOG(LogKSJ, Warning, TEXT("PickupWeapon: Weapon is already picked up by another Troll"));
		return false;
	}

	AAO_Troll* Troll = Cast<AAO_Troll>(GetOwner());
	if (!Troll)
	{
		AO_LOG(LogKSJ, Warning, TEXT("PickupWeapon: Owner is not a Troll"));
		return false;
	}

	// 무기 줍기 실행
	if (!Weapon->PickupByTroll(Troll))
	{
		return false;
	}

	CurrentWeapon = Weapon;

	// 시야 목록에서 제거
	WeaponsInSight.Remove(Weapon);

	// 델리게이트 호출
	OnWeaponPickedUp.Broadcast(Weapon);
	OnWeaponStateChanged.Broadcast(true);

	AO_LOG(LogKSJ, Log, TEXT("%s picked up weapon"), *Troll->GetName());
	return true;
}

void UAO_WeaponHolderComp::DropWeapon()
{
	if (!CurrentWeapon.IsValid())
	{
		return;
	}

	AAO_TrollWeapon* Weapon = CurrentWeapon.Get();
	CurrentWeapon.Reset();

	// 무기 드롭 실행
	Weapon->Drop();

	// 델리게이트 호출
	OnWeaponDropped.Broadcast(Weapon);
	OnWeaponStateChanged.Broadcast(false);

	AO_LOG(LogKSJ, Log, TEXT("Weapon dropped"));
}

AAO_TrollWeapon* UAO_WeaponHolderComp::FindNearestWeaponInSight() const
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return nullptr;
	}

	AAO_TrollWeapon* NearestWeapon = nullptr;
	float NearestDistSq = FLT_MAX;

	for (const TWeakObjectPtr<AAO_TrollWeapon>& WeakWeapon : WeaponsInSight)
	{
		if (!WeakWeapon.IsValid())
		{
			continue;
		}

		AAO_TrollWeapon* Weapon = WeakWeapon.Get();
		
		// 이미 소지 중인 무기 제외
		if (Weapon->IsPickedUp())
		{
			continue;
		}

		const float DistSq = FVector::DistSquared(Owner->GetActorLocation(), Weapon->GetActorLocation());
		if (DistSq < NearestDistSq)
		{
			NearestDistSq = DistSq;
			NearestWeapon = Weapon;
		}
	}

	return NearestWeapon;
}

AAO_TrollWeapon* UAO_WeaponHolderComp::FindNearestWeaponInRadius(float Radius) const
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return nullptr;
	}

	// 월드 내 모든 TrollWeapon 검색
	TArray<AActor*> FoundWeapons;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAO_TrollWeapon::StaticClass(), FoundWeapons);

	// 디버그 로그: 월드에 존재하는 무기 개수 확인
	static double LastLogTime = 0.0;
	double CurrentTime = FPlatformTime::Seconds();
	if (CurrentTime - LastLogTime > 2.0) // 2초마다 로그
	{
		AO_LOG(LogKSJ, Log, TEXT("[DEBUG] FindNearestWeaponInRadius: Found %d weapons in world. Radius: %f"), FoundWeapons.Num(), Radius);
		LastLogTime = CurrentTime;
	}

	AAO_TrollWeapon* NearestWeapon = nullptr;
	float NearestDistSq = FMath::Square(Radius);

	for (AActor* Actor : FoundWeapons)
	{
		AAO_TrollWeapon* Weapon = Cast<AAO_TrollWeapon>(Actor);
		if (!Weapon || Weapon->IsPickedUp())
		{
			continue;
		}

		const float DistSq = FVector::DistSquared(Owner->GetActorLocation(), Weapon->GetActorLocation());
		
		// 거리 체크 디버그 (너무 가까운/먼 경우 확인)
		if (CurrentTime - LastLogTime < 0.1) // 위 로그와 타이밍 맞추기 위해
		{
			// AO_LOG(LogKSJ, Verbose, TEXT("[DEBUG] Check Weapon: %s, Dist: %f"), *Weapon->GetName(), FMath::Sqrt(DistSq));
		}

		if (DistSq < NearestDistSq)
		{
			NearestDistSq = DistSq;
			NearestWeapon = Weapon;
		}
	}

	return NearestWeapon;
}

const TArray<AAO_TrollWeapon*>& UAO_WeaponHolderComp::GetWeaponsInSight() const
{
	// 캐시 갱신
	CachedWeaponsInSight.Reset();
	
	for (const TWeakObjectPtr<AAO_TrollWeapon>& WeakWeapon : WeaponsInSight)
	{
		if (WeakWeapon.IsValid())
		{
			CachedWeaponsInSight.Add(WeakWeapon.Get());
		}
	}

	return CachedWeaponsInSight;
}

void UAO_WeaponHolderComp::OnWeaponDetected(AAO_TrollWeapon* Weapon)
{
	if (!Weapon)
	{
		return;
	}

	// 이미 소지 중인 무기 또는 다른 Troll이 소지 중인 무기 제외
	if (Weapon->IsPickedUp())
	{
		return;
	}

	if (!WeaponsInSight.Contains(Weapon))
	{
		WeaponsInSight.Add(Weapon);
		AO_LOG(LogKSJ, Log, TEXT("Weapon detected: %s"), *Weapon->GetName());
	}
}

void UAO_WeaponHolderComp::OnWeaponLost(AAO_TrollWeapon* Weapon)
{
	if (!Weapon)
	{
		return;
	}

	WeaponsInSight.Remove(Weapon);
	AO_LOG(LogKSJ, Log, TEXT("Weapon lost from sight: %s"), *Weapon->GetName());
}
