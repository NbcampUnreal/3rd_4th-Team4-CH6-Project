// KSJ : AO_GameplayEffect_InsectDamage.h

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "AO_GameplayEffect_InsectDamage.generated.h"

/**
 * Insect 납치 중 데미지 GameplayEffect
 * - 주기적으로 플레이어에게 데미지 적용
 */
UCLASS()
class AO_API UAO_GameplayEffect_InsectDamage : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UAO_GameplayEffect_InsectDamage();
};
