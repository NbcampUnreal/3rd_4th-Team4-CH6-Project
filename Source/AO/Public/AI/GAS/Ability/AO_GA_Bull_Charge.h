#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AO_GA_Bull_Charge.generated.h"

class AAO_Bull;

/**
 * Bull 돌진 Ability (Troll 공격 로직 참조)
 * 
 * - Bull의 SetIsCharging(true) 호출
 * - 몽타주(돌진) 재생
 * - Notify(Event.Combat.Confirm) 수신 시 전방 SphereTrace로 적 감지
 * - 감지된 적에게 넉백 적용 및 넉다운 이벤트 전송
 */
UCLASS()
class AO_API UAO_GA_Bull_Charge : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UAO_GA_Bull_Charge();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();

    // [Troll 참조] 히트 판정 이벤트 처리
    UFUNCTION()
    void OnHitConfirmEvent(FGameplayEventData Payload);

    // [Troll 참조] 넉다운 이벤트 발송
    void SendKnockdownEvent(AActor* TargetActor, AActor* InstigatorActor);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> ChargeMontage;

    // 공격 판정 범위
    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float TraceRadius = 100.f;

    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float TraceDistance = 150.f;

    // 데미지 
    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float ChargeDamage = 30.f;

    // 넉백 강도
    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float KnockbackStrength = 1000.f;

    // 넉다운 HitReact 태그
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    FGameplayTag KnockdownHitReactTag;
};
