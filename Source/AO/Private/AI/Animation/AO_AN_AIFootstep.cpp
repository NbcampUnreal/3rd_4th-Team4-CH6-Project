//KSJ : AO_AN_AIFootstep - AI 캐릭터용 범용 사운드 AnimNotify

#include "AI/Animation/AO_AN_AIFootstep.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundAttenuation.h"
#include "Components/AudioComponent.h"

UAO_AN_AIFootstep::UAO_AN_AIFootstep()
{
	VolumeMultiplier = 1.0f;
	PitchMultiplier = 1.0f;
	MaxDuration = 0.0f;
}

void UAO_AN_AIFootstep::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp || !Sound)
	{
		return;
	}

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner)
	{
		return;
	}

	// Attenuation 설정이 있으면 SpawnSoundAtLocation 사용 (3D 공간화 + 리버브 지원)
	if (AttenuationSettings)
	{
		UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(
			Owner,
			Sound,
			Owner->GetActorLocation(),
			FRotator::ZeroRotator,
			VolumeMultiplier,
			PitchMultiplier,
			0.0f,
			AttenuationSettings
		);

		// MaxDuration이 설정되어 있으면 해당 시간 후 자동 중지
		if (AudioComp && MaxDuration > 0.0f)
		{
			AudioComp->StopDelayed(MaxDuration);
		}
	}
	else
	{
		// Attenuation 없으면 기본 PlaySoundAtLocation 사용
		UGameplayStatics::PlaySoundAtLocation(
			Owner,
			Sound,
			Owner->GetActorLocation(),
			FRotator::ZeroRotator,
			VolumeMultiplier,
			PitchMultiplier
		);
	}
}

FString UAO_AN_AIFootstep::GetNotifyName_Implementation() const
{
	if (Sound)
	{
		return FString::Printf(TEXT("AI Sound: %s"), *Sound->GetName());
	}
	return TEXT("AI Sound");
}
