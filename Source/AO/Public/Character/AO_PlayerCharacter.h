// AO_PlayerCharacter.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AO_PlayerCharacter_MovementEnums.h"
#include "AbilitySystemInterface.h"
#include "AO_PlayerCharacter.generated.h"

class UMotionWarpingComponent;
class UAO_TraversalComponent;
class UCameraComponent;
class USpringArmComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class UAbilitySystemComponent;
class UAO_InteractionComponent;
class UAO_InspectionComponent;

USTRUCT(BlueprintType)
struct FCharacterInputState
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerCharacter|Input")
	bool bWantsToSprint = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerCharacter|Input")
	bool bWantsToWalk = false;
};

UCLASS()
class AO_API AAO_PlayerCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AAO_PlayerCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void Landed(const FHitResult& Hit) override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

public:
	FORCEINLINE USpringArmComponent* GetSpringArm() const {	return SpringArm; }
	FORCEINLINE UCameraComponent* GetCamera() const { return Camera; }

	// 승조 : Inspect하는 중인지 확인
	UFUNCTION(BlueprintPure, Category = "PlayerCharacter|Inspection")
	bool IsInspecting() const;
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacter|Components")
	TObjectPtr<USpringArmComponent> SpringArm;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacter|Components")
	TObjectPtr<UCameraComponent> Camera;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|Components")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|Components")
	TObjectPtr<UAO_InteractionComponent> InteractionComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|Components")
	TObjectPtr<UAO_TraversalComponent> TraversalComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|Components")
	TObjectPtr<UMotionWarpingComponent> MotionWarpingComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|Components")
	TObjectPtr<UAO_InspectionComponent> InspectionComponent;
	//ms: inventory component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	class UAO_InventoryComponent* InventoryComp;
	
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacter|Input")
	TObjectPtr<UInputAction> IA_Select_inventory_Slot;
		
public:
	UPROPERTY(EditAnywhere, Category = "PlayerCharacter|Input")
	FCharacterInputState CharacterInputState;
	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Gait, Category = "PlayerCharacter|Movement")
	EGait Gait = EGait::Run;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "PlayerCharacter|Movement")
	FVector LandVelocity;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "PlayerCharacter|Movement")
	bool bJustLanded = false;

protected:
	UFUNCTION(Server, Reliable)
	void ServerRPC_SetInputState(bool bWantsToSprint, bool bWantsToWalk);
	UFUNCTION()
	void OnRep_Gait();
	
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
	void StartJump();
	void TriggerJump();

	// Movement
	void SetCurrentGait();
	
	//ms: inventory component input
	void SelectInventorySlot(const FInputActionValue& Value);
	
};
