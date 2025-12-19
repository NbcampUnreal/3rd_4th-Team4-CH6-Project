// AO_GA_Troll_Attack.h

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AI/Character/AO_Troll.h"
#include "AO_GA_Troll_Attack.generated.h"

/**
 * Troll 공격 전용 Gameplay Ability
 * 
 * 역할:
 * - 랜덤 공격 타입 선택
 * - 공격 몽타주 재생
 * - 데미지 및 넉백 적용
 * - HitReact 이벤트 발송
 */
UCLASS()
class AO_API UAO_GA_Troll_Attack : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UAO_GA_Troll_Attack();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

	virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags,
		const FGameplayTagContainer* TargetTags,
		FGameplayTagContainer* OptionalRelevantTags) const override;

protected:
	// 히트 이벤트 처리
	UFUNCTION()
	void OnHitConfirmEvent(FGameplayEventData Payload);

	// 데미지 및 넉백 적용
	void ApplyDamageAndKnockback(AActor* TargetActor, const FAO_TrollAttackConfig& Config);

	// HitReact 이벤트 발송
	void SendKnockdownEvent(AActor* TargetActor, AActor* InstigatorActor);

	// 몽타주 종료 콜백
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();

protected:
	// 현재 공격 설정
	FAO_TrollAttackConfig CurrentAttackConfig;

	// 현재 공격 타입
	ETrollAttackType CurrentAttackType = ETrollAttackType::HorizontalSingle;

	// 데미지 Effect 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|AI|Troll")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	// 넉다운 HitReact 태그
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|AI|Troll")
	FGameplayTag KnockdownHitReactTag;

	// 트레이스 채널
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|AI|Troll")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Pawn;
};
