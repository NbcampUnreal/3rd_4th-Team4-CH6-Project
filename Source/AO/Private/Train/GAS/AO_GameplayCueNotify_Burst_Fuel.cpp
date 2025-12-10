#include "Train/GAS/AO_GameplayCueNotify_Burst_Fuel.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UAO_GameplayCueNotify_Burst_Fuel::HandleGameplayCue(
	AActor* Target,
	EGameplayCueEvent::Type EventType,
	const FGameplayCueParameters& Parameters
)
{
	// EventType branch
	if (EventType == EGameplayCueEvent::Executed)
	{		
		/*
		UKismetSystemLibrary::PrintString(
			Target,
			TEXT("FuelAdd Cue Executed!"),
			true, true,
			FLinearColor::Yellow,
			1.5f
		);
		*/

		if (EventType == EGameplayCueEvent::Executed)
		{
			//UKismetSystemLibrary::PrintString(Target, TEXT("FuelAdd Cue Executed!"));

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
}
