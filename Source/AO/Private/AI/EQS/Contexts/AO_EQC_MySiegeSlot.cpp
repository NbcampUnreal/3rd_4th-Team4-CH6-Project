// KSJ : AO_EQC_MySiegeSlot.cpp

#include "AI/EQS/Contexts/AO_EQC_MySiegeSlot.h"
#include "AI/Controller/AO_AIC_Werewolf.h"
#include "AI/Manager/AO_Manager_WerewolfPackManager.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Point.h"
#include "NavigationSystem.h"

void UAO_EQC_MySiegeSlot::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	AActor* Querier = Cast<AActor>(QueryInstance.Owner.Get());
	if (!Querier) return;

	AAO_AIC_Werewolf* WolfAIC = Querier ? Cast<AAO_AIC_Werewolf>(Querier->GetInstigatorController()) : nullptr;
	if (!WolfAIC) return;

	UWorld* World = Querier->GetWorld();
	if (!World) return;

	UAO_Manager_WerewolfPackManager* PackManager = World->GetSubsystem<UAO_Manager_WerewolfPackManager>();
	if (!PackManager) return;

	FVector SlotDirection;
	if (PackManager->GetAssignedSlotDirection(WolfAIC, SlotDirection))
	{
		AActor* Target = PackManager->GetSiegeTargetFor(WolfAIC);
		if (Target)
		{
			FVector TargetLocation = Target->GetActorLocation();
			
			FVector IdealLocation = TargetLocation + (SlotDirection * 600.0f);
			
			UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
			if (NavSys)
			{
				FNavLocation NavLocation;
				if (NavSys->ProjectPointToNavigation(IdealLocation, NavLocation, FVector(500.0f, 500.0f, 500.0f)))
				{
					IdealLocation = NavLocation.Location;
				}
			}
			
			UEnvQueryItemType_Point::SetContextHelper(ContextData, IdealLocation);
		}
	}
}

