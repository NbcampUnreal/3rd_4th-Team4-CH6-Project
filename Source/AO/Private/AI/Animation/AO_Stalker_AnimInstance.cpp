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

