// AO_TrollController.cpp

#include "AI/Controller/AO_TrollController.h"
#include "AI/Character/AO_Troll.h"
#include "AI/Component/AO_WeaponHolderComp.h"
#include "AI/Item/AO_TrollWeapon.h"
#include "AO_Log.h"

AAO_TrollController::AAO_TrollController()
{
}

void AAO_TrollController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AAO_Troll* Troll = Cast<AAO_Troll>(InPawn);
	if (Troll)
	{
		AO_LOG(LogKSJ, Log, TEXT("TrollController possessed %s"), *Troll->GetName());
	}
}

AAO_Troll* AAO_TrollController::GetTroll() const
{
	return Cast<AAO_Troll>(GetPawn());
}

bool AAO_TrollController::TryPickupWeapon()
{
	AAO_Troll* Troll = GetTroll();
	if (!Troll)
	{
		return false;
	}

	UAO_WeaponHolderComp* WeaponHolder = Troll->GetWeaponHolderComponent();
	if (!WeaponHolder)
	{
		return false;
	}

	// 이미 무기를 들고 있으면 줍지 않음
	if (WeaponHolder->HasWeapon())
	{
		return false;
	}

	// 시야 내 가장 가까운 무기 찾기
	AAO_TrollWeapon* NearestWeapon = WeaponHolder->FindNearestWeaponInSight();
	
	// 시야에 없으면 반경 내 검색
	if (!NearestWeapon)
	{
		NearestWeapon = WeaponHolder->FindNearestWeaponInRadius(WeaponSearchRadius);
	}

	if (!NearestWeapon)
	{
		return false;
	}

	// 무기 줍기 시도
	return WeaponHolder->PickupWeapon(NearestWeapon);
}

bool AAO_TrollController::ShouldPrioritizeWeaponPickup() const
{
	AAO_Troll* Troll = GetTroll();
	if (!Troll)
	{
		return false;
	}

	// 이미 무기를 들고 있으면 우선순위 없음
	if (Troll->HasWeapon())
	{
		return false;
	}

	// 이미 무기 줍기 중이면 유지
	if (Troll->IsPickingUpWeapon())
	{
		return true;
	}

	// 플레이어가 공격 범위 내에 있으면 밟기 공격 가능하므로 무기 우선 아님
	if (Troll->IsTargetInAttackRange())
	{
		return false;
	}

	// 시야에 무기가 있는지 확인
	return HasWeaponInSight();
}

bool AAO_TrollController::CanAttack() const
{
	AAO_Troll* Troll = GetTroll();
	if (!Troll)
	{
		return false;
	}

	// 기절 중이면 공격 불가
	if (Troll->IsStunned())
	{
		return false;
	}

	// 이미 공격 중이면 불가
	if (Troll->IsAttacking())
	{
		return false;
	}

	// 무기 줍기 중이면 불가
	if (Troll->IsPickingUpWeapon())
	{
		return false;
	}

	// 대상이 공격 범위 내에 있어야 함
	return Troll->IsTargetInAttackRange();
}

FVector AAO_TrollController::GetNearestWeaponLocation() const
{
	AAO_Troll* Troll = GetTroll();
	if (!Troll)
	{
		return FVector::ZeroVector;
	}

	UAO_WeaponHolderComp* WeaponHolder = Troll->GetWeaponHolderComponent();
	if (!WeaponHolder)
	{
		return FVector::ZeroVector;
	}

	// 시야 내 가장 가까운 무기
	AAO_TrollWeapon* Weapon = WeaponHolder->FindNearestWeaponInSight();
	if (!Weapon)
	{
		// 반경 내 검색
		Weapon = WeaponHolder->FindNearestWeaponInRadius(WeaponSearchRadius);
	}

	if (Weapon)
	{
		return Weapon->GetActorLocation();
	}

	return FVector::ZeroVector;
}

bool AAO_TrollController::HasWeaponInSight() const
{
	AAO_Troll* Troll = GetTroll();
	if (!Troll)
	{
		return false;
	}

	UAO_WeaponHolderComp* WeaponHolder = Troll->GetWeaponHolderComponent();
	if (!WeaponHolder)
	{
		return false;
	}

	return WeaponHolder->GetWeaponsInSight().Num() > 0;
}

void AAO_TrollController::OnActorDetected(AActor* Actor, const FVector& Location)
{
	Super::OnActorDetected(Actor, Location);

	// TrollWeapon 감지
	AAO_TrollWeapon* Weapon = Cast<AAO_TrollWeapon>(Actor);
	if (Weapon)
	{
		AAO_Troll* Troll = GetTroll();
		if (Troll)
		{
			UAO_WeaponHolderComp* WeaponHolder = Troll->GetWeaponHolderComponent();
			if (WeaponHolder)
			{
				WeaponHolder->OnWeaponDetected(Weapon);
			}
		}
	}
}

void AAO_TrollController::OnActorLost(AActor* Actor)
{
	Super::OnActorLost(Actor);

	// TrollWeapon 시야 이탈
	AAO_TrollWeapon* Weapon = Cast<AAO_TrollWeapon>(Actor);
	if (Weapon)
	{
		AAO_Troll* Troll = GetTroll();
		if (Troll)
		{
			UAO_WeaponHolderComp* WeaponHolder = Troll->GetWeaponHolderComponent();
			if (WeaponHolder)
			{
				WeaponHolder->OnWeaponLost(Weapon);
			}
		}
	}
}
