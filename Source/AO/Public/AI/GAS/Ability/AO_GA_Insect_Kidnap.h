// AO_GA_Insect_Kidnap.h

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AO_GA_Insect_Kidnap.generated.h"

class AAO_Insect;

/**
 * Insect 납치 시도 Gameplay Ability
 * 
 * - 몽타주 재생
 * - 히트 판정 (Sphere Trace)
 * - KidnapComponent를 통해 납치 실행
 */
UCLASS()
class AO_API UAO_GA_Insect_Kidnap : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UAO_GA_Insect_Kidnap();

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

	// 히트 이벤트 처리
	UFUNCTION()
	void OnHitConfirmEvent(FGameplayEventData Payload);

	// 몽타주 종료 콜백
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();

	// 납치 Trace 수행 (이벤트 또는 타이머에서 호출)
	void PerformKidnapTrace();

protected:
	// 납치 시도 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|AI|Insect")
	TObjectPtr<UAnimMontage> KidnapMontage;

	// 트레이스 반경
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|AI|Insect")
	float TraceRadius = 100.f;

	// 트레이스 거리
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|AI|Insect")
	float TraceDistance = 150.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|AI|Insect")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Pawn;

	// Trace 타이머 핸들
	FTimerHandle TraceTimerHandle;
};

