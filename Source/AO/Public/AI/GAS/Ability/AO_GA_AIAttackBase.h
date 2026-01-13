//KSJ : AO_GA_AIAttackBase

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AI/AO_AITypes.h"
#include "AO_GA_AIAttackBase.generated.h"

/**
 * 모든 AI(적)의 공통 근접 공격 Ability
 * 
 * - AAO_AICharacterBase의 GetCurrentAttackConfig()를 통해 공격 데이터(데미지, 몽타주 등)를 가져옴
 * - NotifyState(Event.Combat.Confirm) 타이밍에 맞춰 Sphere Trace 수행
 * - 데미지 및 넉백 적용
 */
UCLASS()
class AO_API UAO_GA_AIAttackBase : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UAO_GA_AIAttackBase();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

protected:
	// 히트 확정 이벤트 처리 (AnimNotify)
	UFUNCTION()
	void OnHitConfirmEvent(FGameplayEventData Payload);

	// 몽타주 완료 처리
	UFUNCTION()
	void OnMontageCompleted();

	// 몽타주 취소 처리
	UFUNCTION()
	void OnMontageCancelled();

	// 데미지 및 넉백 적용
	virtual void ApplyDamageAndKnockback(AActor* TargetActor, AActor* InstigatorActor, const FEnemyAttackConfig& Config);

	// 히트 반응 이벤트 전송
	virtual void SendHitReactEvent(AActor* TargetActor, AActor* InstigatorActor, float Damage);

	// 자식 클래스에서 오버라이드 가능한 히트 콜백 (레거시 지원 또는 확장용)
	virtual void OnTargetHit(AActor* TargetActor, AActor* InstigatorActor);

protected:
	// 현재 수행 중인 공격 설정
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FEnemyAttackConfig CurrentAttackConfig;

	// 트레이스 채널 (기본값: Pawn)
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Pawn;

	// 넉다운 히트 리액트 태그 (기본값: Event.Combat.HitReact.Knockdown)
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	FGameplayTag KnockdownHitReactTag;
    
    // 일반 히트 리액트 태그 (기본값: Event.Combat.HitReact.Light)
    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    FGameplayTag DefaultHitReactTag;

	// 타격음
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	TObjectPtr<USoundBase> HitSound;

	// 타격음 감쇠 설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	TObjectPtr<USoundAttenuation> HitSoundAttenuation;

	// 타격음 멀티캐스트 재생
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayHitSound(const FVector& Location);
};
