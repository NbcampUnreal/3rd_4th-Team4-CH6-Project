// HSJ : AO_GameplayAbility_Interact_Execute.h
#pragma once

#include "CoreMinimal.h"
#include "AO_GameplayAbility_Interact_Info.h"
#include "GameplayAbilitySpecHandle.h"
#include "AO_GameplayAbility_Interact_Execute.generated.h"

/**
 * 상호작용 실행 어빌리티
 * - Duration 홀딩 관리
 * - 몽타주 재생 및 노티파이 처리
 * - DecayHoldElement는 자체 관리
 */
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

	void OnAnimNotifyReceived(const FGameplayEventData* EventData);

	UFUNCTION(Server, Reliable)
	void ServerNotifyDecayHoldInputReleased();

	UPROPERTY(EditDefaultsOnly, Category="AO|Interaction")
	float AcceptanceAngle = 120.f;

	UPROPERTY(EditDefaultsOnly, Category="AO|Interaction")
	float AcceptanceDistance = 100.f;

private:
	FTimerHandle DurationTimerHandle;
	FTimerHandle MontageTimerHandle;
	bool bHoldingPhaseCompleted;
};