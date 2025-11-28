// KSJ : AO_Manager_CrabPickupManager.cpp

#include "AI/Manager/AO_Manager_CrabPickupManager.h"
#include "AI/Character/AO_Enemy_Crab.h"
#include "AI/Item/AO_PickupComponent.h"
#include "Engine/World.h"
#include "AO_Log.h"

void UAO_Manager_CrabPickupManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	AO_LOG(LogKSJ, Log, TEXT("[CrabPickupManager] Initialized"));
	
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(
			CooldownCleanupTimerHandle,
			this,
			&UAO_Manager_CrabPickupManager::CleanupExpiredCooldowns,
			CooldownCleanupInterval,
			true
		);
	}
}

void UAO_Manager_CrabPickupManager::Deinitialize()
{
	UWorld* World = GetWorld();
	if (World && CooldownCleanupTimerHandle.IsValid())
	{
		World->GetTimerManager().ClearTimer(CooldownCleanupTimerHandle);
	}
	
	ItemOwnershipMap.Empty();
	ItemCooldownMap.Empty();
	
	Super::Deinitialize();
}

bool UAO_Manager_CrabPickupManager::CanPickupItem(AActor* ItemActor) const
{
	if (!ItemActor) return false;
	
	if (ItemOwnershipMap.Contains(ItemActor))
	{
		return false;
	}
	
	const double* CooldownEndTime = ItemCooldownMap.Find(ItemActor);
	if (CooldownEndTime)
	{
		double CurrentTime = FPlatformTime::Seconds();
		if (CurrentTime < *CooldownEndTime)
		{
			return false;
		}
	}
	
	return true;
}

bool UAO_Manager_CrabPickupManager::TryClaimItem(AActor* ItemActor, AAO_Enemy_Crab* ClaimingCrab)
{
	if (!ItemActor || !ClaimingCrab) return false;
	
	if (!CanPickupItem(ItemActor))
	{
		AAO_Enemy_Crab* CurrentOwner = GetItemOwner(ItemActor);
		if (CurrentOwner)
		{
			AO_LOG(LogKSJ, Verbose, TEXT("[CrabPickupManager] Item %s is already owned by %s"), 
				*ItemActor->GetName(), *CurrentOwner->GetName());
		}
		return false;
	}
	
	ItemOwnershipMap.Add(ItemActor, ClaimingCrab);
	ItemCooldownMap.Remove(ItemActor);
	
	AO_LOG(LogKSJ, Log, TEXT("[CrabPickupManager] Item %s claimed by %s"), 
		*ItemActor->GetName(), *ClaimingCrab->GetName());
	
	return true;
}

void UAO_Manager_CrabPickupManager::ReleaseItem(AActor* ItemActor)
{
	if (!ItemActor) return;
	
	AAO_Enemy_Crab* PreviousOwner = ItemOwnershipMap.FindRef(ItemActor);
	if (PreviousOwner)
	{
		ItemOwnershipMap.Remove(ItemActor);
		
		double CooldownEndTime = FPlatformTime::Seconds() + ItemCooldownDuration;
		ItemCooldownMap.Add(ItemActor, CooldownEndTime);
		
		AO_LOG(LogKSJ, Log, TEXT("[CrabPickupManager] Item %s released by %s (Cooldown: %.1fs)"), 
			*ItemActor->GetName(), *PreviousOwner->GetName(), ItemCooldownDuration);
	}
}

AAO_Enemy_Crab* UAO_Manager_CrabPickupManager::GetItemOwner(AActor* ItemActor) const
{
	if (!ItemActor) return nullptr;
	
	return ItemOwnershipMap.FindRef(ItemActor);
}

AActor* UAO_Manager_CrabPickupManager::GetCrabHeldItem(AAO_Enemy_Crab* Crab) const
{
	if (!Crab) return nullptr;
	
	for (const auto& Pair : ItemOwnershipMap)
	{
		if (Pair.Value == Crab)
		{
			return Pair.Key;
		}
	}
	
	return nullptr;
}

void UAO_Manager_CrabPickupManager::CleanupExpiredCooldowns()
{
	double CurrentTime = FPlatformTime::Seconds();
	
	for (auto It = ItemCooldownMap.CreateIterator(); It; ++It)
	{
		if (CurrentTime >= It.Value())
		{
			It.RemoveCurrent();
		}
	}
}

