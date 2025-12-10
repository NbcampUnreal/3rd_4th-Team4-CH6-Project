// AO_GameplayAbility_MeleeHitConfirm.h

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Character/Combat/AO_MeleeHitEventPayload.h"
#include "AO_GameplayAbility_MeleeHitConfirm.generated.h"

/**
 * 노티파이에서 보낸 이벤트를 받아 실제 히트 판정과 데미지 적용을 담당.
 */
UCLASS()
class AO_API UAO_GameplayAbility_MeleeHitConfirm : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UAO_GameplayAbility_MeleeHitConfirm();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trace")
	TEnumAsByte<ECollisionChannel> TraceChannel;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trace")
	bool bIgnoreInstigator = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HitReact")
	FGameplayTag HitReactEventTag;

protected:
	void DoMeleeHitConfirm(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayEventData* EventData);
	void ApplyDamageToActor(AActor* TargetActor, const FAO_MeleeHitTraceParams& Params, const FGameplayAbilityActorInfo* ActorInfo);
	void SendHitReactEvent(AActor* TargetActor, AActor* InstigatorActor, float DamageAmount);
};
