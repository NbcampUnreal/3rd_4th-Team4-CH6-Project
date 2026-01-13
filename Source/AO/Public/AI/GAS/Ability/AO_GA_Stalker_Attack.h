//KSJ : AO_GA_Stalker_Attack

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AO_GA_Stalker_Attack.generated.h"

/**
 * Stalker 기습 공격 Ability
 * - 몽타주 재생 및 히트 판정
 * - 데미지 및 HitReact 적용
 */
UCLASS()
class AO_API UAO_GA_Stalker_Attack : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UAO_GA_Stalker_Attack();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();

	// 히트 이벤트 처리
	UFUNCTION()
	void OnHitConfirmEvent(FGameplayEventData Payload);

	// 데미지 및 넉백 적용
	void ApplyDamageAndKnockback(AActor* TargetActor);

	// HitReact 이벤트 발송
	void SendHitReactEvent(AActor* TargetActor, AActor* InstigatorActor);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> AttackMontage;

	// 데미지 Effect 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	// 공격 데미지
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float AttackDamage = 30.f;

	// 넉백 강도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float KnockbackStrength = 500.f;

	// HitReact 태그 (플레이어에게 보낼 태그)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	FGameplayTag HitReactTag;

	// 트레이스 채널
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Pawn;

	// 타격음
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	TObjectPtr<USoundBase> HitSound;

	// 타격음 감쇠 설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	TObjectPtr<USoundAttenuation> HitSoundAttenuation;
};
