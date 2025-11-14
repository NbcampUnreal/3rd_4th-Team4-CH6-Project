// HSJ : GA_Interact_Base.h
#pragma once

#include "CoreMinimal.h"
#include "Interaction/GAS/Ability/AO_InteractionGameplayAbility.h"
#include "GA_Interact_Base.generated.h"

/**
 * 범용 상호작용 어빌리티
 * 
 * 동작:
 * 1. Target 추출
 * 2. Target->OnInteractionSuccess(Instigator) 호출
 * 
 * 사용처:
 * - AAO_BaseInteractable 기본 어빌리티
 * - 대부분의 간단한 상호작용
 */
UCLASS()
class AO_API UGA_Interact_Base : public UAO_InteractionGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Interact_Base();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
};