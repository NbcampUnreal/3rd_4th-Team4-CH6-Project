// AO_Troll_AnimInstance.cpp

#include "AI/Animation/AO_Troll_AnimInstance.h"
#include "AI/Character/AO_Troll.h"
#include "GameFramework/CharacterMovementComponent.h"

UAO_Troll_AnimInstance::UAO_Troll_AnimInstance()
{
}

void UAO_Troll_AnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	UpdateCharacterReference();
}

void UAO_Troll_AnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// 캐릭터 참조가 없으면 다시 시도
	if (!OwningTroll.IsValid())
	{
		UpdateCharacterReference();
	}

	// 애니메이션 속성 업데이트
	UpdateAnimationProperties();
}

void UAO_Troll_AnimInstance::PlayStunMontage()
{
	if (StunMontage)
	{
		// 이미 재생 중이면 무시
		if (!Montage_IsPlaying(StunMontage))
		{
			Montage_Play(StunMontage, StunMontagePlayRate);
		}
	}
}

void UAO_Troll_AnimInstance::StopStunMontage(float BlendOutTime)
{
	if (StunMontage && Montage_IsPlaying(StunMontage))
	{
		Montage_Stop(BlendOutTime, StunMontage);
	}
}

void UAO_Troll_AnimInstance::PlayPickupWeaponMontage()
{
	if (PickupWeaponMontage)
	{
		if (!Montage_IsPlaying(PickupWeaponMontage))
		{
			Montage_Play(PickupWeaponMontage, PickupWeaponMontagePlayRate);
		}
	}
}

void UAO_Troll_AnimInstance::UpdateCharacterReference()
{
	if (APawn* PawnOwner = TryGetPawnOwner())
	{
		OwningTroll = Cast<AAO_Troll>(PawnOwner);
	}
}

void UAO_Troll_AnimInstance::UpdateAnimationProperties()
{
	if (!OwningTroll.IsValid())
	{
		return;
	}

	AAO_Troll* Troll = OwningTroll.Get();

	// === Locomotion 업데이트 ===
	if (UCharacterMovementComponent* MovementComp = Troll->GetCharacterMovement())
	{
		// 2D 속도 (XY 평면)
		Speed = MovementComp->Velocity.Size2D();
		bIsMoving = Speed > 10.f;
	}

	// === 상태 플래그 업데이트 ===
	bIsStunned = Troll->IsStunned();
	bIsAttacking = Troll->IsAttacking();
	bHasWeapon = Troll->HasWeapon();
	bIsPickingUpWeapon = Troll->IsPickingUpWeapon();
}
