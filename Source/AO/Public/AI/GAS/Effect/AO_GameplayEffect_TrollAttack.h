// KSJ : AO_GameplayEffect_TrollAttack.h

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "AO_GameplayEffect_TrollAttack.generated.h"

/**
 * 트롤 공격 효과 (데미지 + 넉백 태그)
 * - Health 감소 (SetByCaller: Data.Damage)
 * - Stun 태그 부여 (옵션)
 */
UCLASS()
class AO_API UAO_GameplayEffect_TrollAttack : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UAO_GameplayEffect_TrollAttack();
};