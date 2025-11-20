// HSJ : GA_Interact_PuzzleElement.h
#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Interaction/GAS/Ability/AO_InteractionGameplayAbility.h"
#include "GA_Interact_PuzzleElement.generated.h"

/**
 * 퍼즐 요소 상호작용 어빌리티
 * 
 * 1. Execute 어빌리티가 Finalize 이벤트 발생
 * 2. 이 어빌리티가 자동 활성화 (AbilityTrigger)
 * 3. TriggerEventData의 Target(PuzzleElement)에 OnInteractionSuccess() 호출
 * 
 * 주의:
 * - ServerOnly 실행
 * - Finalize 태그로만 트리거됨
 * - PuzzleElement의 실제 로직 실행 담당
 */
UCLASS()
class AO_API UGA_Interact_PuzzleElement : public UAO_InteractionGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Interact_PuzzleElement();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
};