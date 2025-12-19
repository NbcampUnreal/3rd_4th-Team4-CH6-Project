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

	// 부모 클래스에서 GroundSpeed와 bShouldMove를 업데이트했으므로, 이를 블루프린트용 변수로 매핑
	Speed = GroundSpeed;
	bIsMoving = bShouldMove;
}

