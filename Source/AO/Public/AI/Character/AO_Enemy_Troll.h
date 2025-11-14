// AO_Enemy_Troll.h

#pragma once

#include "CoreMinimal.h"
#include "AI/Character/AO_Enemy_HumanoidBase.h"
#include "AO_Enemy_Troll.generated.h"

UCLASS()
class AO_API AAO_Enemy_Troll : public AAO_Enemy_HumanoidBase
{
	GENERATED_BODY()

public:
	AAO_Enemy_Troll();

protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Combat")
	float KnockbackPower = 600.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Combat")
	float AttackCoolDown = 3.0f;

public:
	FORCEINLINE float GetKnockbackPower() const { return KnockbackPower; }
	FORCEINLINE float GetAttackCooldown() const { return AttackCoolDown; }
};