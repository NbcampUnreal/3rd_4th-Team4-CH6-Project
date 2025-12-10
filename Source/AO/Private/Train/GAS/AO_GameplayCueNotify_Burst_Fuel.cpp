#include "Train/GAS/AO_GameplayCueNotify_Burst_Fuel.h"
#include "Kismet/GameplayStatics.h"

void UAO_GameplayCueNotify_Burst_Fuel::HandleGameplayCue(
	AActor* Target,
	EGameplayCueEvent::Type EventType,
	const FGameplayCueParameters& Parameters
)
{
	if (EventType == EGameplayCueEvent::Executed)
	{		
		FVector SpawnLocation = Parameters.Location;
		if (SpawnLocation.IsNearlyZero() && Target)
		{
			SpawnLocation = Target->GetActorLocation();
		}
		UGameplayStatics::SpawnEmitterAtLocation(
			Target->GetWorld(),
			FuelParticles,
			SpawnLocation
			);
		UGameplayStatics::PlaySoundAtLocation(
			Target,
			FuelSound,
			SpawnLocation
		);
	}
}

