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

	// 상호작용 실행
	bool ExecuteInteraction();

	// Task 콜백
	UFUNCTION()
	void OnInvalidInteraction();

	UFUNCTION()
	void OnDurationEnded();

	UFUNCTION()
	void OnInteractionInputReleased();

	UPROPERTY(EditDefaultsOnly, Category="AO|Interaction")
	float AcceptanceAngle = 65.f;

	UPROPERTY(EditDefaultsOnly, Category="AO|Interaction")
	float AcceptanceDistance = 10.f;

	FTimerHandle DurationTimerHandle;
};