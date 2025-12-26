// 사용법
// level에 스폰하고
// 캐릭터가 선택한 인벤토리에 MasterItem을 가진 채로 상호작용시 연료를 채우고 소모됨.

// 연료 소비 시작 함수 FuelLeakSkillOn()
// 연료 소비 종료 함수 FuelLeakSkillOff()

#pragma once

#include "CoreMinimal.h"
#include "GAS/AO_Fuel_AttributeSet.h"
#include "Interaction/Base/AO_BaseInteractable.h"
#include "AO_newTrain.generated.h"

UCLASS()
class AO_API AAO_newTrain : public AAO_BaseInteractable
{
	GENERATED_BODY()

public:
	AAO_newTrain();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|Abilities")
	void FuelLeakSkillOn();
	UFUNCTION(BlueprintCallable, Category = "GAS|Abilities")
	void FuelLeakSkillOut();
	
	UPROPERTY()
	TArray<TWeakObjectPtr<UObject>> FuelListeners;
	void BindFuel(UObject* Listener);
	UFUNCTION(BlueprintCallable, Category="Train|UI")
	void BindFuelListener(UObject* Listener);
	void HandleFuelAttributeChanged(const FOnAttributeChangeData& Data);
protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> ASC;
	
	UPROPERTY()
	const UAO_Fuel_AttributeSet* FuelAttributeSet;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayAbility> AddEnergyAbilityClass;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayAbility> LeakEnergyAbilityClass;
	virtual void OnInteractionSuccess(AActor* Interactor) override;
};
