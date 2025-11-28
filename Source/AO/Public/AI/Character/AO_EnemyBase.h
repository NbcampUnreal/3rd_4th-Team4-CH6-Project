// KSJ : AO_EnemyBase.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "AO_EnemyBase.generated.h"

class UAbilitySystemComponent;
class UAO_Enemy_AttributeSet;

/**
 * GAS를 사용하는 모든 적 AI의 Base 클래스
 *
 * - AbilitySystemComponent
 * - AO_Enemy_AttributeSet
 * - MovementSpeed 변동시 CharacterMovement 적용
 * - Health 변동시 기절(Stun) 처리
 */

UCLASS()
class AO_API AAO_EnemyBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AAO_EnemyBase();

	// IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UAO_Enemy_AttributeSet* GetAttributeSet() const { return AttributeSet; }

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_Controller() override;

	virtual void OnMovementSpeedChanged(const struct FOnAttributeChangeData& Data);
	virtual void OnHealthChanged(const struct FOnAttributeChangeData& Data);
	virtual void OnStunned();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAO_Enemy_AttributeSet> AttributeSet;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Abilities")
	TArray<TSubclassOf<class UGameplayAbility>> DefaultAbilities;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Attributes")
	TSubclassOf<class UGameplayEffect> DefaultAttributes;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsStunned = false;

private:
	void InitAbilitySystem();
	void GiveDefaultAbilities();
	void InitializeAttributes();
};