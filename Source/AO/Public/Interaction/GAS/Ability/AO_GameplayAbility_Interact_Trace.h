// HSJ : AO_GameplayAbility_Interact_Trace.h
#pragma once

#include "CoreMinimal.h"
#include "AO_InteractionGameplayAbility.h"
#include "Interaction/Interface/AO_InteractionInfo.h"
#include "AO_GameplayAbility_Interact_Trace.generated.h"

UCLASS()
class AO_API UAO_GameplayAbility_Interact_Trace : public UAO_InteractionGameplayAbility
{
	GENERATED_BODY()

public:
	UAO_GameplayAbility_Interact_Trace();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	// Task 콜백
	UFUNCTION()
	void UpdateInteractions(const TArray<FAO_InteractionInfo>& InteractionInfos);

	UPROPERTY(EditDefaultsOnly, Category="AO|Interaction")
	float InteractionTraceRange = 150.f;

	UPROPERTY(EditDefaultsOnly, Category="AO|Interaction")
	float InteractionTraceRate = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category="AO|Interaction")
	bool bShowTraceDebug = false;

	UPROPERTY(EditDefaultsOnly, Category="AO|Interaction", meta=(ClampMin="0.0", UIMin="0.0", UIMax="500.0"))
	float TraceSphereRadius = 60.0f;
};