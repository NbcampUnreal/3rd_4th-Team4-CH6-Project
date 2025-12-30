//KSJ : AO_Werewolf_AnimInstance

#include "AI/Animation/AO_Werewolf_AnimInstance.h"
#include "AI/Character/AO_Werewolf.h"
#include "GameFramework/CharacterMovementComponent.h"

void UAO_Werewolf_AnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	WerewolfCharacter = Cast<AAO_Werewolf>(Character);
}

void UAO_Werewolf_AnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (WerewolfCharacter)
	{
		// 직접 속도 계산 (안전장치)
		Velocity = WerewolfCharacter->GetVelocity(); // 멤버 변수 Velocity 업데이트
		GroundSpeed = Velocity.Size2D();

		// 가속도 확인
		bool bHasAcceleration = false;
		if (UCharacterMovementComponent* MoveComp = WerewolfCharacter->GetCharacterMovement())
		{
			bHasAcceleration = MoveComp->GetCurrentAcceleration().SizeSquared2D() > 0.f;
		}

		// 이동 중 판정
		bShouldMove = (GroundSpeed > 3.0f) && bHasAcceleration;

		// 블루프린트용 변수 업데이트
		Speed = GroundSpeed;
		bIsMoving = bShouldMove;
	}
}

