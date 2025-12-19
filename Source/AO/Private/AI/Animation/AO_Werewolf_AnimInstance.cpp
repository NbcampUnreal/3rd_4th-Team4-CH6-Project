#include "AI/Animation/AO_Werewolf_AnimInstance.h"
#include "AI/Character/AO_Werewolf.h"

void UAO_Werewolf_AnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	WerewolfCharacter = Cast<AAO_Werewolf>(Character);
}

void UAO_Werewolf_AnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	// 추가적인 Werewolf 전용 상태 업데이트가 필요하면 여기에 작성
}

