//KSJ : AO_Stalker_AnimInstance

#include "AI/Animation/AO_Stalker_AnimInstance.h"
#include "AI/Character/AO_Stalker.h"
#include "AI/Component/AO_CeilingMoveComponent.h"

void UAO_Stalker_AnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	StalkerCharacter = Cast<AAO_Stalker>(Character);
}

void UAO_Stalker_AnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (StalkerCharacter)
	{
		// CeilingMoveComponent 상태 확인
		if (UAO_CeilingMoveComponent* CeilingComp = StalkerCharacter->GetCeilingMoveComponent())
		{
			bInCeiling = CeilingComp->IsInCeilingMode();
		}

		// 기절 상태 확인
		bIsStunned = StalkerCharacter->IsStunned();

		// 로컬 속도 계산 (월드 속도를 캐릭터 회전 기준으로 변환)
		FVector WorldVelocity = StalkerCharacter->GetVelocity();
		FRotator ActorRotation = StalkerCharacter->GetActorRotation();
		
		// UnrotateVector: 월드 벡터를 로컬 벡터로 변환
		LocalVelocity = ActorRotation.UnrotateVector(WorldVelocity);
		
		ForwardSpeed = LocalVelocity.X;
		RightSpeed = LocalVelocity.Y;
	}
}

UAnimMontage* UAO_Stalker_AnimInstance::GetAttackMontage() const
{
	if (AttackMontages.Num() == 0)
	{
		return nullptr;
	}

	// 랜덤 선택 (나중에 거리/방향 기반으로 고도화 가능)
	int32 Index = FMath::RandRange(0, AttackMontages.Num() - 1);
	return AttackMontages[Index];
}

void UAO_Stalker_AnimInstance::PlayStunMontage()
{
	if (!StunMontage)
	{
		return;
	}

	Montage_Play(StunMontage, StunMontagePlayRate);
}

void UAO_Stalker_AnimInstance::StopStunMontage(float BlendOutTime)
{
	if (StunMontage)
	{
		Montage_Stop(BlendOutTime, StunMontage);
	}
}

