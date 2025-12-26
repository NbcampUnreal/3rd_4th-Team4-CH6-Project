#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AO_GA_AIAttackBase.generated.h"

/**
 * AI 범용 공격 Ability Base
 * 
 * - 몽타주 재생 -> Notify(Event.Combat.Confirm) 대기 -> SphereTrace -> Hit 처리
 * - 자식 클래스에서 OnTargetHit를 오버라이드하여 납치 등 특수 로직 구현 가능
 */
UCLASS()
class AO_API UAO_GA_AIAttackBase : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UAO_GA_AIAttackBase();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
    // 몽타주 재생 함수 (자식에서 오버라이드하여 랜덤 선택 등 구현 가능)
    virtual UAnimMontage* GetMontageToPlay() const;

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();

    // 히트 판정 이벤트 수신
    UFUNCTION()
    void OnHitConfirmEvent(FGameplayEventData Payload);

    // [Virtual] 적중 시 처리 로직 (기본: 데미지+넉백+이벤트)
    virtual void OnTargetHit(AActor* TargetActor, AActor* InstigatorActor);

protected:
    // --- 설정 변수들 ---

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UAnimMontage> AttackMontage;

    // 공격 판정 범위
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    float TraceRadius = 50.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    float TraceDistance = 150.f;

    // 데미지 
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    float DamageAmount = 10.f;

    // 넉백 강도
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    float KnockbackStrength = 0.f;
    
    // 넉백 방향 (UpVector 보정치)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    float KnockbackUpForce = 0.3f;

    // 데미지 GE 클래스
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    TSubclassOf<UGameplayEffect> DamageEffectClass;

    // 적중 시 상대에게 보낼 이벤트 태그 (HitReact/Knockdown 등)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    FGameplayTag HitReactTag;
    
    // 디버그 트레이스 표시 여부
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    bool bShowDebugTrace = false;
};

