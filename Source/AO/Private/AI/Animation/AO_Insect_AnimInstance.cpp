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
	}
}

