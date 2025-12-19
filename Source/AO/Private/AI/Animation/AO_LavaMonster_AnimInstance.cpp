// AO_LavaMonster_AnimInstance.cpp

#include "AI/Animation/AO_LavaMonster_AnimInstance.h"
#include "AI/Character/AO_LavaMonster.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AO_Log.h"

UAO_LavaMonster_AnimInstance::UAO_LavaMonster_AnimInstance()
{
}

void UAO_LavaMonster_AnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	UpdateCharacterReference();
}

void UAO_LavaMonster_AnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	UpdateCharacterReference();
	UpdateAnimationProperties();
}

void UAO_LavaMonster_AnimInstance::UpdateCharacterReference()
{
	if (!OwningLavaMonster.IsValid())
	{
		if (AAO_LavaMonster* LavaMonster = Cast<AAO_LavaMonster>(TryGetPawnOwner()))
		{
			OwningLavaMonster = LavaMonster;
		}
	}
}

void UAO_LavaMonster_AnimInstance::UpdateAnimationProperties()
{
	if (!OwningLavaMonster.IsValid())
	{
		return;
	}

	AAO_LavaMonster* LavaMonster = OwningLavaMonster.Get();

	// 기절 상태
	bIsStunned = LavaMonster->IsStunned();

	// 공격 상태
	bIsAttacking = LavaMonster->IsAttacking();

	// 이동 중이 아닐 때 Idle 랜덤 재생 (부모 클래스의 bShouldMove 사용)
	bool bIsMoving = false;
	if (UCharacterMovementComponent* MovementComp = LavaMonster->GetCharacterMovement())
	{
		bIsMoving = MovementComp->Velocity.Size2D() > 10.f;
	}

	if (!bIsMoving && !bIsStunned && !bIsAttacking)
	{
		// 타이머가 설정되지 않았으면 설정
		if (!IdleRandomTimerHandle.IsValid() && IdleMontages.Num() > 0)
		{
			float NextInterval = IdleRandomInterval + FMath::RandRange(-IdleRandomIntervalDeviation, IdleRandomIntervalDeviation);
			NextInterval = FMath::Max(0.1f, NextInterval);

			if (UWorld* World = GetWorld())
			{
				World->GetTimerManager().SetTimer(
					IdleRandomTimerHandle,
					this,
					&UAO_LavaMonster_AnimInstance::PlayRandomIdleMontage,
					NextInterval,
					false
				);
			}
		}
	}
	else
	{
		// 이동 중이거나 공격 중이면 Idle 타이머 클리어
		if (IdleRandomTimerHandle.IsValid())
		{
			if (UWorld* World = GetWorld())
			{
				World->GetTimerManager().ClearTimer(IdleRandomTimerHandle);
			}
		}
	}
}

void UAO_LavaMonster_AnimInstance::PlayStunMontage()
{
	if (!StunMontage)
	{
		AO_LOG(LogKSJ, Warning, TEXT("PlayStunMontage: StunMontage is null"));
		return;
	}

	Montage_Play(StunMontage, StunMontagePlayRate);
}

void UAO_LavaMonster_AnimInstance::StopStunMontage(float BlendOutTime)
{
	if (StunMontage)
	{
		Montage_Stop(BlendOutTime, StunMontage);
	}
}

void UAO_LavaMonster_AnimInstance::PlayRandomIdleMontage()
{
	if (IdleMontages.Num() == 0)
	{
		return;
	}

	// 이전 Idle 몽타주가 재생 중이면 중지
	if (CurrentIdleMontage)
	{
		Montage_Stop(0.1f, CurrentIdleMontage);
	}

	// 랜덤 선택
	const int32 RandomIndex = FMath::RandRange(0, IdleMontages.Num() - 1);
	CurrentIdleMontage = IdleMontages[RandomIndex];

	if (CurrentIdleMontage)
	{
		Montage_Play(CurrentIdleMontage, IdleMontagePlayRate);
		
		// 다음 Idle 재생 타이머 설정
		float NextInterval = IdleRandomInterval + FMath::RandRange(-IdleRandomIntervalDeviation, IdleRandomIntervalDeviation);
		NextInterval = FMath::Max(0.1f, NextInterval);

		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(
				IdleRandomTimerHandle,
				this,
				&UAO_LavaMonster_AnimInstance::PlayRandomIdleMontage,
				NextInterval,
				false
			);
		}
	}
}

