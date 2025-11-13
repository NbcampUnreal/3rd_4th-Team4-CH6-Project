// AO_PlayerCharacter.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AO_PlayerCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class UAbilitySystemComponent;
class UAO_InteractionComponent;

UCLASS()
class AO_API AAO_PlayerCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AAO_PlayerCharacter();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	FORCEINLINE USpringArmComponent* GetSpringArm() const {	return SpringArm; }
	FORCEINLINE UCameraComponent* GetCamera() const { return Camera; }
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacter|Components")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacter|Components")
	TObjectPtr<UCameraComponent> Camera;

	// 승조 : ASC
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|Components")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	// 승조 : 상호작용 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|Components")
	TObjectPtr<UAO_InteractionComponent> InteractionComponent;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacter|Input")
	TObjectPtr<UInputMappingContext> IMC_Player;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacter|Input")
	TObjectPtr<UInputAction> IA_Move;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacter|Input")
	TObjectPtr<UInputAction> IA_Look;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacter|Input")
	TObjectPtr<UInputAction> IA_Jump;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacter|Input")
	TObjectPtr<UInputAction> IA_Sprint;

private:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartSprint();
	void StopSprint();
};
