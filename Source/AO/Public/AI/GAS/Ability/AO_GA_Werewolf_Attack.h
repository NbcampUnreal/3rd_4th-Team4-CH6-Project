// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/GAS/Ability/AO_GA_AIAttackBase.h"
#include "AO_GA_Werewolf_Attack.generated.h"

/**
 * Werewolf Attack Ability
 * - 근접 공격 (Heavy Hit)
 * - AO_GA_AIAttackBase를 상속하여 히트 처리 로직 사용
 */
UCLASS()
class AO_API UAO_GA_Werewolf_Attack : public UAO_GA_AIAttackBase
{
	GENERATED_BODY()

public:
	UAO_GA_Werewolf_Attack();
	
protected:
	// AO_GA_AIAttackBase의 OnTargetHit 오버라이드 (Heavy Hit React 적용)
	virtual void OnTargetHit(AActor* TargetActor, AActor* InstigatorActor) override;
};
