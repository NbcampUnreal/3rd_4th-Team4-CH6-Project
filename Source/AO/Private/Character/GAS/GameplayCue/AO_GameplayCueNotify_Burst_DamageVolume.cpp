// AO_GameplayCueNotify_Burst_DamageVolume.cpp


#include "Character/GAS/GameplayCue/AO_GameplayCueNotify_Burst_DamageVolume.h"

#include "AbilitySystemComponent.h"
#include "AO_Log.h"
#include "Character/Sound/AO_PlayerSoundSubsystem.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

bool UAO_GameplayCueNotify_Burst_DamageVolume::OnExecute_Implementation(AActor* MyTarget,
	const FGameplayCueParameters& Parameters) const
{
	if (!MyTarget)
	{
		return false;
	}

	// 사망 시에는 사망 소리 재생
	if (UAbilitySystemComponent* ASC = MyTarget->FindComponentByClass<UAbilitySystemComponent>())
	{
		if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Status.Death"))))
		{
			return false;
		}
	}
	
	PlayDamageReactSound(MyTarget);
	return true;
}

void UAO_GameplayCueNotify_Burst_DamageVolume::PlayDamageReactSound(AActor* Target) const
{
	UGameInstance* GI = Target->GetGameInstance();
	checkf(GI, TEXT("Failed to get GI"));

	UAO_PlayerSoundSubsystem* SoundSubsystem = GI->GetSubsystem<UAO_PlayerSoundSubsystem>();
	checkf(SoundSubsystem, TEXT("Failed to get SoundSubsystem"));

	USoundBase* DamageReactSound = SoundSubsystem->GetSoundFromActor(Target,
		FGameplayTag::RequestGameplayTag(FName("Sound.Player.DamageReact")));
	if (!ensure(DamageReactSound))
	{
		return;
	}

	if (const APawn* Pawn = Cast<APawn>(Target))
	{
		const AController* Controller = Pawn->GetController();
		const bool bIsLocal = Controller && Controller->IsLocalController();

		if (bIsLocal)
		{
			UGameplayStatics::PlaySound2D(Target, DamageReactSound);
			return;
		}
	}

	if (ACharacter* Character = Cast<ACharacter>(Target))
	{
		if (USkeletalMeshComponent* MeshComp = Character->GetMesh())
		{
			UGameplayStatics::SpawnSoundAttached(DamageReactSound, MeshComp, NAME_None);
			return;
		}
	}

	UGameplayStatics::PlaySoundAtLocation(Target, DamageReactSound, Target->GetActorLocation());
}
