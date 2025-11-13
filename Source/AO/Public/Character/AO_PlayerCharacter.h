// AO_PlayerCharacter.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AO_PlayerCharacter_MovementEnums.h"
#include "AO_PlayerCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

USTRUCT(BlueprintType)
struct FCharacterInputState
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacter|Input")
	bool bWantsToSprint = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacter|Input")
	bool bWantsToWalk = false;
};

UCLASS()
class AO_API AAO_PlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AAO_PlayerCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void Landed(const FHitResult& Hit) override;

public:
	FORCEINLINE USpringArmComponent* GetSpringArm() const {	return SpringArm; }
	FORCEINLINE UCameraComponent* GetCamera() const { return Camera; }
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacter|Components")
	TObjectPtr<USpringArmComponent> SpringArm;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacter|Components")
	TObjectPtr<UCameraComponent> Camera;
	
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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacter|Input")
	TObjectPtr<UInputAction> IA_Crouch;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacter|Input")
	TObjectPtr<UInputAction> IA_Walk;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerCharacter|Input")
	FCharacterInputState CharacterInputState;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerCharacter|Movement")
	EGait Gait = EGait::Run;
	UPROPERTY(BlueprintReadOnly, Category = "PlayerCharacter|Movement")
	FVector LandVelocity;
	UPROPERTY(BlueprintReadOnly, Category = "PlayerCharacter|Movement")
	bool bJustLanded = false;

private:
	FTimerHandle TimerHandle_JustLanded;
	
private:
	// Input Actions
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartSprint();
	void StopSprint();
	void HandleCrouch();
	void HandleWalk();

	// Movement
	void SetCurrentGait();
};
