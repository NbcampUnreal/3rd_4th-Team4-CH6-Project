// HSJ : AO_GameplayAbility_Inspect_Enter.h
#pragma once

#include "CoreMinimal.h"
#include "Interaction/GAS/Ability/AO_InteractionGameplayAbility.h"
#include "AO_GameplayAbility_Inspect_Enter.generated.h"

/**
 * Inspection 모드 진입 어빌리티
 * 
 * - Inspection 모드가 필요한 액터의 AbilityToGrant로 에디터에서 설정
 * - GA_Interact_Execute에서 Finalize 이벤트 시 TriggerEventData->Target에서 액터 가져옴
 * - InspectionComponent->EnterInspectionMode()를 호출
 * - Status.Action.Inspecting 태그 부여
 */
UCLASS()
class AO_API UAO_GameplayAbility_Inspect_Enter : public UAO_InteractionGameplayAbility
{
	GENERATED_BODY()

public:
	UAO_GameplayAbility_Inspect_Enter();

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
};