// AO_PlayerCharacter.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AO_PlayerCharacter_MovementEnums.h"
#include "AbilitySystemInterface.h"
#include "Foley/AO_FoleyAudioBankInterface.h"
#include "Item/invenroty/AO_InventoryComponent.h"
#include "Item/PassiveContainer/AO_PassiveComponent.h"
#include "Net/VoiceConfig.h"				// JM : VOIPTalker
#include "AO_PlayerCharacter.generated.h"

class UAO_DeathSpectateComponent;
class UAO_PlayerCharacter_AttributeDefaults;
class UAO_CustomizingComponent;
class UCustomizableObjectInstance;
class UAO_PlayerCharacter_AttributeSet;
class UGameplayAbility;
class UGameplayEffect;
class UAttributeSet;
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
class UAO_InteractableComponent;
class UAO_FoleyAudioBank;
class UCustomizableSkeletalComponent;
class UAIPerceptionStimuliSourceComponent;

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
class AO_API AAO_PlayerCharacter : public ACharacter, public IAbilitySystemInterface, public IAO_FoleyAudioBankInterface
{
	GENERATED_BODY()

public:
	AAO_PlayerCharacter();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
protected:
	virtual void PossessedBy(AController* NewController) override;
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void Landed(const FHitResult& Hit) override;
	virtual void OnJumped_Implementation() override;

	virtual UAO_FoleyAudioBank* GetFoleyAudioBank_Implementation() const override;
	virtual bool CanPlayFootstepSounds_Implementation() const override;

	virtual void CalcCamera(float DeltaTime, struct FMinimalViewInfo& OutResult) override;

public:
	FORCEINLINE TObjectPtr<USpringArmComponent> GetSpringArm() const {	return SpringArm; }
	FORCEINLINE TObjectPtr<UCameraComponent> GetCamera() const { return Camera; }
	FORCEINLINE TObjectPtr<UAO_PlayerCharacter_AttributeSet> GetAttributeSet() const { return AttributeSet; }
	FORCEINLINE TObjectPtr<UAO_InteractableComponent> GetInteractableComponent() const { return InteractableComponent; }
	
	// 승조 : Inspect하는 중인지 확인
	UFUNCTION(BlueprintPure, Category = "PlayerCharacter|Inspection")
	bool IsInspecting() const;

	void StartSprint_GAS(bool bShouldSprint);
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacter|Components")
	TObjectPtr<USpringArmComponent> SpringArm;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacter|Components")
	TObjectPtr<UCameraComponent> Camera;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacter|Components")	// JM : VOIPTalker
	TObjectPtr<UVOIPTalker> VOIPTalker;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|Components")
	TObjectPtr<UAO_InteractionComponent> InteractionComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|Components")
	TObjectPtr<UMotionWarpingComponent> MotionWarpingComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|Components")
	TObjectPtr<UAO_InspectionComponent> InspectionComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|Components")
	TObjectPtr<UAO_InteractableComponent> InteractableComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|Components")
	TObjectPtr<UAO_DeathSpectateComponent> DeathSpectateComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	TObjectPtr<UAO_InventoryComponent> InventoryComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	TObjectPtr<UAO_PassiveComponent> PassiveComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|Components")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacter|GAS")
	TObjectPtr<UAO_PlayerCharacter_AttributeSet> AttributeSet;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacter|GAS")
	TObjectPtr<UAO_PlayerCharacter_AttributeDefaults> AttributeDefaults;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacter|GAS")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacter|GAS")
	TMap<int32, TSubclassOf<UGameplayAbility>> InputAbilities;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacter|GAS")
	TArray<TSubclassOf<UGameplayEffect>> DefaultEffects;

	//세훈: Customizable Object Instance
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|Components")
	TObjectPtr<USkeletalMeshComponent> BaseSkeletalMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|Components")
	TObjectPtr<USkeletalMeshComponent> BodySkeletalMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|Components")
	TObjectPtr<USkeletalMeshComponent> HeadSkeletalMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|Components")
	TObjectPtr<UCustomizableSkeletalComponent> BodyComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|Components")
	TObjectPtr<UCustomizableSkeletalComponent> HeadComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|Components")
	TObjectPtr<UAO_CustomizingComponent> CustomizingComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|Components")
	TObjectPtr<UAIPerceptionStimuliSourceComponent> AIPerceptionStimuliSource;
	
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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacter|Foley")
	TObjectPtr<UAO_FoleyAudioBank> DefaultFoleyAudioBank;

public:
	UPROPERTY(EditAnywhere, Category = "PlayerCharacter|Input")
	FCharacterInputState CharacterInputState;
	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Gait, Category = "PlayerCharacter|Movement")
	EGait Gait = EGait::Run;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "PlayerCharacter|Movement")
	FVector LandVelocity;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "PlayerCharacter|Movement")
	bool bJustLanded = false;
	UPROPERTY(EditDefaultsOnly, Category = "PlayerCharacter|Movement")
	float DeathCameraArmOffset = 300.f;

protected:
	UFUNCTION(Server, Reliable)
	void ServerRPC_SetInputState(bool bWantsToSprint, bool bWantsToWalk);
	UFUNCTION()
	void OnRep_Gait();

	// HSJ : InteractableComponent의 상호작용 성공 시 호출될 함수
	UFUNCTION()
	void HandleInteractableComponentSuccess(AActor* Interactor);
	
private:
	FTimerHandle TimerHandle_JustLanded;
	FTimerHandle VOIPRegisterToPSTimerHandle;	// JM : VOIPTalker
	
private:
	// Input Actions
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void HandleCrouch();
	void HandleWalk();
	void StartJump();
	void TriggerJump();
	void HandleGameplayAbilityInputPressed(int32 InInputID);
	void HandleGameplayAbilityInputReleased(int32 InInputID);

	// Movement
	void SetCurrentGait();

	// Foley
	void PlayAudioEvent(FGameplayTag Value, float VolumeMultiplier = 1.0f, float PitchMultiplier = 1.0f);

	// Bind GAS
	void InitializeAttributes();
	void BindGameplayAbilities();
	void BindGameplayEffects();
	void BindAttributeDelegates();
	void BindSpeedAttributeDelegates();

	// Speed
	void OnSpeedChanged(const FOnAttributeChangeData& Data);
	
// JM : VOIPTalker Register to PS
private:
	void TryRegisterVoiceTalker();
	void RegisterVoiceTalker();
	void InitVoiceChat();
	
public:
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "AO|VoiceChat")
	TObjectPtr<USoundAttenuation> SA_VoiceChat = nullptr;

//세훈: Customizable Object Instance
	TObjectPtr<UCustomizableSkeletalComponent> GetBodyComponent() const { return BodyComponent; }
	TObjectPtr<UCustomizableSkeletalComponent> GetHeadComponent() const { return HeadComponent; }
	TObjectPtr<UAO_CustomizingComponent> GetCustomizingComponent() const { return CustomizingComponent; }

//ms : 사망시 delegate로 불리는 함수
	void HandlePlayerDeath();
};
