
// 사용법
// level에 스폰하고
// 캐릭터가 선택한 인벤토리에 MasterItem을 가진 채로 상호작용시 연료를 채우고 소모됨.

// 연료 소비 시작 함수 FuelLeakSkillOn()
// 연료 소비 종료 함수 FuelLeakSkillOff()

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Actor.h"
#include "Abilities/GameplayAbility.h"
#include "Interaction/Component/AO_InteractableComponent.h"
#include "AO_Train.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFuelChanged, float, NewFuel);

class UAbilitySystemComponent;
class UAO_Fuel_AttributeSet;

UCLASS()
class AO_API AAO_Train : public AActor,  public IAbilitySystemInterface

{
	GENERATED_BODY()

public:
	AAO_Train();
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
	TObjectPtr<UStaticMeshComponent> StaticMesh;
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayAbility> AddEnergyAbilityClass;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayAbility> LeakEnergyAbilityClass;

	UFUNCTION()
	void HandleInteractionSuccess(AActor* Interactor);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interact")
	class UAO_InteractableComponent* InteractableComp;

	UPROPERTY(BlueprintAssignable, Category="Fuel")
	FOnFuelChanged OnFuelChangedDelegate;

protected:
	virtual void BeginPlay() override;
	
	void OnFuelChanged(const FOnAttributeChangeData& Data);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> ASC;

	UPROPERTY()
	UAO_Fuel_AttributeSet* FuelAttributeSet;

	UFUNCTION(BlueprintCallable, Category = "GAS|Abilities")
	void FuelLeakSkillOn();
	UFUNCTION(BlueprintCallable, Category = "GAS|Abilities")
	void FuelLeakSkillOut();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fuel")
	float TotalFuelGained = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fuel")
	float LeakFuelAmount = 10.f; // 연료 감소량
};
