// HSJ : AO_InteractionComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AO_InteractionComponent.generated.h"

class UAbilitySystemComponent;
class UGameplayAbility;
class UInputAction;
class UAO_InteractionWidget;
class UAO_InteractionWidgetController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractInputReleased);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AO_API UAO_InteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAO_InteractionComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetupInputBinding(UInputComponent* PlayerInputComponent);

	// 상호작용하는 타겟 관리
	void SetCurrentInteractTarget(AActor* Target) { CurrentInteractTarget = Target; }
	AActor* GetCurrentInteractTarget() const { return CurrentInteractTarget; }

	UFUNCTION(Server, Reliable)
	void ServerTriggerInteract(AActor* TargetActor);

	UFUNCTION(Server, Reliable)
	void ServerNotifyInteractReleased();

	// 상호작용 몽타주 재생 멀티캐스트
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayInteractionMontage(UAnimMontage* MontageToPlay);
	
	UFUNCTION(BlueprintPure, Category="Interaction")
	UAO_InteractionWidgetController* GetInteractionWidgetController() const 
	{ 
		return InteractionWidgetController; 
	}

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY(BlueprintReadOnly, Replicated)
	bool bIsHoldingInteract = false;

	// 상호작용 키 뗐을 때 델리게이트
	UPROPERTY(BlueprintAssignable)
	FOnInteractInputReleased OnInteractInputReleased;

protected:
	virtual void BeginPlay() override;

	// UI 초기화
	void InitializeInteractionUI(APlayerController* PC);

	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UAO_InteractionWidget> InteractionWidgetClass;

	UPROPERTY()
	TObjectPtr<UAO_InteractionWidget> InteractionWidget;

	UPROPERTY()
	TObjectPtr<UAO_InteractionWidgetController> InteractionWidgetController;

private:
	void GiveDefaultAbilities();
	void OnInteractPressed();
	void OnInteractReleased();

	UAbilitySystemComponent* GetOwnerAbilitySystemComponent() const;

	// 상호작용하는 타겟
	UPROPERTY()
	TObjectPtr<AActor> CurrentInteractTarget;
};