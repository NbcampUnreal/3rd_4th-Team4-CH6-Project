// HSJ : AO_GameplayAbility_Interact.h
#pragma once

#include "CoreMinimal.h"
#include "AO_InteractionGameplayAbility.h"
#include "AO_GameplayAbility_Interact.generated.h"

UCLASS()
class AO_API UAO_GameplayAbility_Interact : public UAO_InteractionGameplayAbility
{
	GENERATED_BODY()

public:
	UAO_GameplayAbility_Interact();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	UPROPERTY(EditDefaultsOnly, Category="AO|Interaction")
	float InteractionScanRange = 500.f;
	
	UPROPERTY(EditDefaultsOnly, Category="AO|Interaction")
	float InteractionScanRate = 0.2f;
};