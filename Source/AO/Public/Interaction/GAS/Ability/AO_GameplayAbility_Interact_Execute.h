// HSJ : AO_GameplayAbility_Interact_Execute.h
#pragma once

#include "CoreMinimal.h"
#include "AO_GameplayAbility_Interact_Info.h"
#include "GameplayAbilitySpecHandle.h"
#include "AO_GameplayAbility_Interact_Execute.generated.h"

UCLASS()
class AO_API UAO_GameplayAbility_Interact_Execute : public UAO_GameplayAbility_Interact_Info
{
	GENERATED_BODY()

public:
	UAO_GameplayAbility_Interact_Execute();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

	bool ExecuteInteraction();
	bool SendFinalizeEvent();

	UFUNCTION()
	void OnInvalidInteraction();

	UFUNCTION()
	void OnDurationEnded();

	UFUNCTION()
	void OnInteractionInputReleased();

	UPROPERTY(EditDefaultsOnly, Category="AO|Interaction")
	float AcceptanceAngle = 120.f;

	UPROPERTY(EditDefaultsOnly, Category="AO|Interaction")
	float AcceptanceDistance = 100.f;

private:
	void RotateToTarget();
	
	FTimerHandle DurationTimerHandle;
	FTimerHandle MontageTimerHandle;
};