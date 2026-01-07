// AO_GameplayCueNotify_Burst_Death.cpp

#include "Character/GAS/GameplayCue/AO_GameplayCueNotify_Burst_Death.h"

#include "AO_Log.h"
#include "Character/Sound/AO_PlayerSoundSubsystem.h"
#include "Kismet/GameplayStatics.h"

bool UAO_GameplayCueNotify_Burst_Death::OnExecute_Implementation(AActor* MyTarget,
                                                                 const FGameplayCueParameters& Parameters) const
{
	if (!MyTarget)
	{
		return false;
	}

	PlayDeathSound(MyTarget);
	return true;
}

void UAO_GameplayCueNotify_Burst_Death::PlayDeathSound(AActor* Target) const
{
	UGameInstance* GI = Target->GetGameInstance();
	checkf(GI, TEXT("Failed to get GI"));
	
	UAO_PlayerSoundSubsystem* SoundSubsystem = GI->GetSubsystem<UAO_PlayerSoundSubsystem>();
	checkf(SoundSubsystem, TEXT("Failed to get SoundSubsystem"));

	USoundBase* DeathSound = SoundSubsystem->GetSoundFromActor(Target,
		FGameplayTag::RequestGameplayTag(FName("Sound.Player.Death")));
	if (!ensure(DeathSound))
	{
		return;
	}

	if (const APawn* Pawn = Cast<APawn>(Target))
	{
		const AController* Controller = Pawn->GetController();
		const bool bIsLocal = Controller && Controller->IsLocalController();

		if (bIsLocal)
		{
			UGameplayStatics::PlaySound2D(Target, DeathSound);
			return;
		}
	}

	UGameplayStatics::PlaySoundAtLocation(Target, DeathSound, Target->GetActorLocation());
}
