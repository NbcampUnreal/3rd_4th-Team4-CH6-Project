// AO_GameplayAbility_Traversal.h

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Character/Traversal/AO_TraversalTypes.h"
#include "AO_GameplayAbility_Traversal.generated.h"

class UCapsuleComponent;
class UCharacterMovementComponent;
class UChooserTable;

UCLASS()
class AO_API UAO_GameplayAbility_Traversal : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UAO_GameplayAbility_Traversal();

protected:
	virtual void OnAvatarSet(
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilitySpec& Spec) override;

	virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal")
	TObjectPtr<UChooserTable> AnimChooserTable;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal|Debug")
	int32 DrawDebugLevel = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal|Debug")
	float DrawDebugDuration = 5.f;
	UPROPERTY(EditDefaultsOnly, Category = "Traversal|Stamina")
	TSubclassOf<UGameplayEffect> PostSprintNoRegenEffectClass;
	
private:
	FTraversalCheckResult TraversalResult;

	UPROPERTY()
	TObjectPtr<AActor> Owner = nullptr;
	UPROPERTY()
	TObjectPtr<ACharacter> Character = nullptr;
	UPROPERTY()
	TObjectPtr<UCharacterMovementComponent> CharacterMovement = nullptr;
	UPROPERTY()
	TObjectPtr<UCapsuleComponent> CapsuleComponent = nullptr;

private:
	UFUNCTION()
	void OnMontageCompleted();

	bool TryTraversal();
	bool GetTraversalCheckInputs(FTraversalCheckInput& OutTraversalInput);
	bool DetectTraversal();
	bool EvaluateTraversal(TArray<TObjectPtr<UObject>>& EvaluateObjects);
	bool SelectTraversal(const TArray<TObjectPtr<UObject>>& EvaluateObjects);

	void UpdateWarpTargets();

	bool RunCapsuleTrace(
		const FVector& StartLocation,
		const FVector& EndLocation,
		float Radius,
		float HalfHeight,
		FHitResult& OutHit,
		FColor DebugHitColor,
		FColor DebugTraceColor);
};
