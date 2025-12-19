#include "AI/StateTree/Condition/AO_STCond_TrollWeaponNearby.h"
#include "AI/Character/AO_Troll.h"
#include "AI/Component/AO_WeaponHolderComp.h"
#include "AI/Controller/AO_TrollController.h"
#include "AI/Item/AO_TrollWeapon.h"
#include "StateTreeExecutionContext.h"
#include "AO_Log.h"

bool FAO_STCond_TrollWeaponNearby::TestCondition(FStateTreeExecutionContext& Context) const
{
	// [DEBUG] 함수 호출 여부 확인 (항상 출력)
	AO_LOG(LogKSJ, Warning, TEXT("[DEBUG] STCond_TrollWeaponNearby::TestCondition CALLED"));

	const FAO_STCond_TrollWeaponNearby_InstanceData& InstanceData = Context.GetInstanceData<FAO_STCond_TrollWeaponNearby_InstanceData>(*this);

	AAO_Troll* Troll = GetTroll(Context);
	if (!Troll)
	{
		AO_LOG(LogKSJ, Warning, TEXT("[DEBUG] STCond_TrollWeaponNearby: Troll is null"));
		return InstanceData.bInvert;
	}

	// 이미 무기를 들고 있으면 False (주울 필요 없음)
	if (Troll->HasWeapon())
	{
		AO_LOG(LogKSJ, Log, TEXT("[DEBUG] STCond_TrollWeaponNearby: Troll already has weapon, returning false"));
		return InstanceData.bInvert;
	}

	bool bWeaponNearby = false;
	UAO_WeaponHolderComp* WeaponHolder = Troll->GetWeaponHolderComponent();
	
	if (WeaponHolder)
	{
		// 1. 시야 감지 우선 (가장 빠름)
		AAO_TrollWeapon* FoundWeapon = WeaponHolder->FindNearestWeaponInSight();
		if (FoundWeapon)
		{
			// 너무 자주 찍히지 않게 1초 간격으로 로그
			static double LastLogTime_Sight = 0.0;
			double CurrentTime = FPlatformTime::Seconds();
			if (CurrentTime - LastLogTime_Sight > 1.0)
			{
				AO_LOG(LogKSJ, Log, TEXT("[DEBUG] STCond: Found weapon in SIGHT: %s"), *FoundWeapon->GetName());
				LastLogTime_Sight = CurrentTime;
			}
		}

		// 2. 시야에 없다면 거리 기반 검색 (옵션)
		if (!FoundWeapon && InstanceData.SearchRadius > 0.f)
		{
			FoundWeapon = WeaponHolder->FindNearestWeaponInRadius(InstanceData.SearchRadius);
			
			// 디버그 로그 (1초 간격)
			static double LastLogTime_Radius = 0.0;
			double CurrentTime = FPlatformTime::Seconds();
			if (CurrentTime - LastLogTime_Radius > 1.0)
			{
				if (FoundWeapon)
				{
					AO_LOG(LogKSJ, Log, TEXT("[DEBUG] STCond: Found weapon in RADIUS: %s (Radius: %f)"), *FoundWeapon->GetName(), InstanceData.SearchRadius);
				}
				else
				{
					AO_LOG(LogKSJ, Warning, TEXT("[DEBUG] STCond: No weapon found in Radius %f"), InstanceData.SearchRadius);
				}
				LastLogTime_Radius = CurrentTime;
			}
		}

		bWeaponNearby = (FoundWeapon != nullptr);
	}
	else
	{
		AO_LOG(LogKSJ, Error, TEXT("[DEBUG] STCond: WeaponHolder is null"));
	}

	return InstanceData.bInvert ? !bWeaponNearby : bWeaponNearby;
}

AAO_Troll* FAO_STCond_TrollWeaponNearby::GetTroll(FStateTreeExecutionContext& Context) const
{
	AActor* Owner = Cast<AActor>(Context.GetOwner());
	if (!Owner)
	{
		return nullptr;
	}

	if (AAO_Troll* Troll = Cast<AAO_Troll>(Owner))
	{
		return Troll;
	}
	
	if (AAO_TrollController* Controller = Cast<AAO_TrollController>(Owner))
	{
		return Controller->GetTroll();
	}
	
	if (APawn* Pawn = Cast<APawn>(Owner))
	{
		if (AAO_TrollController* Controller = Cast<AAO_TrollController>(Pawn->GetController()))
		{
			return Controller->GetTroll();
		}
		return Cast<AAO_Troll>(Pawn);
	}

	return nullptr;
}

