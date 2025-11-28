// KSJ : AO_Manager_InsectKidnapManager.cpp

#include "AI/Manager/AO_Manager_InsectKidnapManager.h"
#include "Engine/World.h"
#include "AO_Log.h"

void UAO_Manager_InsectKidnapManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	AO_LOG(LogKSJ, Log, TEXT("[InsectKidnapManager] Initialized"));
	
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(
			CooldownCleanupTimerHandle,
			this,
			&UAO_Manager_InsectKidnapManager::CleanupExpiredCooldowns,
			CooldownCleanupInterval,
			true
		);
	}
}

void UAO_Manager_InsectKidnapManager::Deinitialize()
{
	UWorld* World = GetWorld();
	if (World && CooldownCleanupTimerHandle.IsValid())
	{
		World->GetTimerManager().ClearTimer(CooldownCleanupTimerHandle);
	}
	
	KidnapCooldownMap.Empty();
	KidnappedPlayers.Empty();
	
	Super::Deinitialize();
}

bool UAO_Manager_InsectKidnapManager::CanKidnapPlayer(AActor* PlayerActor) const
{
	if (!PlayerActor) return false;
	
	if (KidnappedPlayers.Contains(PlayerActor))
	{
		return false;
	}
	
	const double* CooldownEndTime = KidnapCooldownMap.Find(PlayerActor);
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

void UAO_Manager_InsectKidnapManager::RegisterKidnapCooldown(AActor* PlayerActor)
{
	if (!PlayerActor) return;
	
	double CooldownEndTime = FPlatformTime::Seconds() + KidnapCooldownDuration;
	KidnapCooldownMap.Add(PlayerActor, CooldownEndTime);
	
	AO_LOG(LogKSJ, Log, TEXT("[InsectKidnapManager] Cooldown registered for %s (Duration: %.1fs)"), 
		*PlayerActor->GetName(), KidnapCooldownDuration);
}

void UAO_Manager_InsectKidnapManager::ClearKidnapCooldown(AActor* PlayerActor)
{
	if (!PlayerActor) return;
	
	KidnapCooldownMap.Remove(PlayerActor);
	AO_LOG(LogKSJ, Log, TEXT("[InsectKidnapManager] Cooldown cleared for %s"), *PlayerActor->GetName());
}

bool UAO_Manager_InsectKidnapManager::IsPlayerKidnapped(AActor* PlayerActor) const
{
	if (!PlayerActor) return false;
	return KidnappedPlayers.Contains(PlayerActor);
}

void UAO_Manager_InsectKidnapManager::RegisterKidnap(AActor* PlayerActor)
{
	if (!PlayerActor) return;
	
	KidnappedPlayers.Add(PlayerActor);
	AO_LOG(LogKSJ, Log, TEXT("[InsectKidnapManager] Kidnap registered for %s"), *PlayerActor->GetName());
}

void UAO_Manager_InsectKidnapManager::UnregisterKidnap(AActor* PlayerActor)
{
	if (!PlayerActor) return;
	
	KidnappedPlayers.Remove(PlayerActor);
	AO_LOG(LogKSJ, Log, TEXT("[InsectKidnapManager] Kidnap unregistered for %s"), *PlayerActor->GetName());
}

void UAO_Manager_InsectKidnapManager::CleanupExpiredCooldowns()
{
	double CurrentTime = FPlatformTime::Seconds();
	
	for (auto It = KidnapCooldownMap.CreateIterator(); It; ++It)
	{
		if (CurrentTime >= It.Value())
		{
			It.RemoveCurrent();
		}
	}
}
