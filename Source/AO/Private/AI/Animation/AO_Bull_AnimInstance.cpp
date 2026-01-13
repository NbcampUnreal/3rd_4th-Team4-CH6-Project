//KSJ : AO_Bull_AnimInstance

#include "AI/Animation/AO_Bull_AnimInstance.h"
#include "AI/Character/AO_Bull.h"

void UAO_Bull_AnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	BullCharacter = Cast<AAO_Bull>(Character);
}

void UAO_Bull_AnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (BullCharacter)
	{
		bIsCharging = BullCharacter->IsCharging();
		bIsStunned = BullCharacter->IsStunned();
	}
}

void UAO_Bull_AnimInstance::PlayStunMontage()
{
	if (!StunMontage)
	{
		return;
	}

	Montage_Play(StunMontage, StunMontagePlayRate);
}

void UAO_Bull_AnimInstance::StopStunMontage(float BlendOutTime)
{
	if (StunMontage)
	{
		Montage_Stop(BlendOutTime, StunMontage);
	}
}

