// AO_Crab_AnimInstance.cpp

#include "AI/Animation/AO_Crab_AnimInstance.h"

#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "AI/Character/AO_Crab.h"
#include "AI/Component/AO_ItemCarryComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

UAO_Crab_AnimInstance::UAO_Crab_AnimInstance()
{
}

void UAO_Crab_AnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	UpdateCharacterReference();
}

void UAO_Crab_AnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// 캐릭터 참조가 없으면 다시 시도
	if (!OwningCrab.IsValid())
	{
		UpdateCharacterReference();
	}

	// 애니메이션 속성 업데이트
	UpdateAnimationProperties();
}

void UAO_Crab_AnimInstance::PlayStunMontage()
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

void UAO_Crab_AnimInstance::StopStunMontage(float BlendOutTime)
{
	if (StunMontage && Montage_IsPlaying(StunMontage))
	{
		Montage_Stop(BlendOutTime, StunMontage);
	}
}

void UAO_Crab_AnimInstance::UpdateCharacterReference()
{
	if (APawn* PawnOwner = TryGetPawnOwner())
	{
		OwningCrab = Cast<AAO_Crab>(PawnOwner);
	}
}

void UAO_Crab_AnimInstance::UpdateAnimationProperties()
{
	if (!OwningCrab.IsValid())
	{
		return;
	}

	AAO_Crab* Crab = OwningCrab.Get();

	// 속도 계산 (수평 속도만)
	if (UCharacterMovementComponent* MovementComp = Crab->GetCharacterMovement())
	{
		FVector Velocity = MovementComp->Velocity;
		Speed = FVector(Velocity.X, Velocity.Y, 0.f).Size();
	}
	else
	{
		Speed = Crab->GetVelocity().Size2D();
	}

	// 이동 중 여부
	bIsMoving = Speed > MovingThreshold;

	// 기절 상태 (GameplayTag 기반)
	if (UAbilitySystemComponent* ASC = Crab->GetAbilitySystemComponent())
	{
		bIsStunned = ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Status.Debuff.Stunned")));
	}
	else
	{
		bIsStunned = false;
	}

	// 아이템 운반 상태
	if (UAO_ItemCarryComponent* ItemCarryComp = Crab->GetItemCarryComponent())
	{
		bIsCarryingItem = ItemCarryComp->IsCarryingItem();
	}
	else
	{
		bIsCarryingItem = false;
	}

	// 도망 상태
	bIsFleeing = Crab->IsInFleeMode();
}
