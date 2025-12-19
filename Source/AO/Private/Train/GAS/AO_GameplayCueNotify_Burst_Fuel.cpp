#include "Train/GAS/AO_GameplayCueNotify_Burst_Fuel.h"
#include "Kismet/GameplayStatics.h"
#include "Train/AO_newTrain.h"

void UAO_GameplayCueNotify_Burst_Fuel::HandleGameplayCue(
	AActor* Target,
	EGameplayCueEvent::Type EventType,
	const FGameplayCueParameters& Parameters
)
{
	if (EventType == EGameplayCueEvent::Executed)
	{		
		FVector SpawnLocation = Parameters.Location;
		if (AAO_newTrain* ParentTarget = Cast<AAO_newTrain>(Target))
		{
			if (ParentTarget->InteractableMeshComponent)
			{
				SpawnLocation =
					ParentTarget->InteractableMeshComponent->GetComponentLocation();
			}
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

