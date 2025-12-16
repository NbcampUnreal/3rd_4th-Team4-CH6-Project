// AO_PlayerCharacter_AttributeSet.h

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AO_PlayerCharacter_AttributeSet.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnPlayerDeath);

UCLASS()
class AO_API UAO_PlayerCharacter_AttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UAO_PlayerCharacter_AttributeSet();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

public:
	// 체력
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Health")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS_BASIC(UAO_PlayerCharacter_AttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Health")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS_BASIC(UAO_PlayerCharacter_AttributeSet, MaxHealth)

	// 스태미나
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Stamina, Category = "Stamina")
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS_BASIC(UAO_PlayerCharacter_AttributeSet, Stamina)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxStamina, Category = "Stamina")
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS_BASIC(UAO_PlayerCharacter_AttributeSet, MaxStamina)

	// 걷기 이동속도
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_WalkSpeed, Category = "Speed")
	FGameplayAttributeData WalkSpeed;
	ATTRIBUTE_ACCESSORS_BASIC(UAO_PlayerCharacter_AttributeSet, WalkSpeed)
	
	// 뛰기(Run) 이동속도
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_RunSpeed, Category = "Speed")
	FGameplayAttributeData RunSpeed;
	ATTRIBUTE_ACCESSORS_BASIC(UAO_PlayerCharacter_AttributeSet, RunSpeed)
	
	// 스프린트(Sprint) 이동속도
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_SprintSpeed, Category = "Speed")
	FGameplayAttributeData SprintSpeed;
	ATTRIBUTE_ACCESSORS_BASIC(UAO_PlayerCharacter_AttributeSet, SprintSpeed)

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lockout")
	float StaminaLockoutPercent = 0.25f;
	
	FOnPlayerDeath OnPlayerDeath;

protected:
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_Stamina(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_MaxStamina(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_WalkSpeed(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_RunSpeed(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_SprintSpeed(const FGameplayAttributeData& OldValue);

private:
	void HandleStaminaLockout(const struct FGameplayEffectModCallbackData& Data);
};
