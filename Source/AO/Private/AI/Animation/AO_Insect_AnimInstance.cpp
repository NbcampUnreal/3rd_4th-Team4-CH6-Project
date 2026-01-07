//KSJ : AO_Insect_AnimInstance

#include "AI/Animation/AO_Insect_AnimInstance.h"
#include "AI/Character/AO_Insect.h"

void UAO_Insect_AnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	InsectCharacter = Cast<AAO_Insect>(Character);
}

void UAO_Insect_AnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (InsectCharacter)
	{
		bIsKidnapping = InsectCharacter->IsKidnapping();
		bIsStunned = InsectCharacter->IsStunned();
	}
}

void UAO_Insect_AnimInstance::PlayStunMontage()
{
	if (!StunMontage)
	{
		return;
	}

	Montage_Play(StunMontage, StunMontagePlayRate);
}

void UAO_Insect_AnimInstance::StopStunMontage(float BlendOutTime)
{
	if (StunMontage)
	{
		Montage_Stop(BlendOutTime, StunMontage);
	}
}

