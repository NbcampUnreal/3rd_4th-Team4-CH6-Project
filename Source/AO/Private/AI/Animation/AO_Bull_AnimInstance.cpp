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

