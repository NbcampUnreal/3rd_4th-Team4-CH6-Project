//KSJ : AO_AITypes

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AO_AITypes.generated.h"

class UAnimMontage;
class UGameplayEffect;

/**
 * 공통 AI 공격 설정 구조체
 * 모든 적 캐릭터(AI)의 공격 데이터 표준화
 */
USTRUCT(BlueprintType)
struct FEnemyAttackConfig
{
	GENERATED_BODY()

	// 공격 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	TObjectPtr<UAnimMontage> AttackMontage = nullptr;

	// 데미지
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	float Damage = 30.f;

	// 넉백 강도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	float KnockbackStrength = 500.f;

	// 공격 범위 (SphereTrace 반경)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	float AttackRadius = 150.f;

	// 공격 거리 (시전 가능 거리)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	float AttackDistance = 200.f;

	// 무기 필요 여부 (Troll 등)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	bool bRequiresWeapon = false;

	// 데미지 Effect 클래스 (선택 사항, 없으면 기본값 사용)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	TSubclassOf<UGameplayEffect> DamageEffectClass;
    
    // 이 공격에 매칭되는 태그 (예: Ability.Combat.Melee.Light)
    // GAS 어빌리티 실행 시 식별용
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
    FGameplayTag AttackTag;
};

