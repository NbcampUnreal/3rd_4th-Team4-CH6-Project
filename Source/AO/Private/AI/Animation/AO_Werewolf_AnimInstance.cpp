//KSJ : AO_Werewolf_AnimInstance

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

	// 부모 클래스가 이미 Velocity, GroundSpeed, bShouldMove를 설정했으므로
	// Werewolf는 블루프린트용 변수만 업데이트
	if (WerewolfCharacter)
	{
		// 블루프린트용 변수 업데이트 (부모 클래스의 변수 사용)
		Speed = GroundSpeed;
		bIsMoving = bShouldMove;
		bIsStunned = WerewolfCharacter->IsStunned();
	}
}

void UAO_Werewolf_AnimInstance::PlayStunMontage()
{
	if (!StunMontage)
	{
		return;
	}

	Montage_Play(StunMontage, StunMontagePlayRate);
}

void UAO_Werewolf_AnimInstance::StopStunMontage(float BlendOutTime)
{
	if (StunMontage)
	{
		Montage_Stop(BlendOutTime, StunMontage);
	}
}

