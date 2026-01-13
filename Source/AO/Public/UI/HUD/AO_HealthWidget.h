// AO_HealthWidget.h

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/AO_UserWidget.h"
#include "AO_HealthWidget.generated.h"

class UAbilitySystemComponent;
struct FOnAttributeChangeData;

UCLASS()
class AO_API UAO_HealthWidget : public UAO_UserWidget
{
	GENERATED_BODY()

public:
	virtual bool Initialize() override;

	UFUNCTION(BlueprintCallable, Category = "GAS|UI")
	void BindToASC(UAbilitySystemComponent* InASC);

	UFUNCTION(BlueprintCallable, Category = "GAS|UI")
	void BindToCharacter(ACharacter* InCharacter);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "GAS|UI")
	void OnHealthChanged(float NewHealth, float NewMaxHealth);

private:
	void BindHealthDelegates(UAbilitySystemComponent* ASC);
	void UnbindHealthDelegates();
	
	void HealthChanged(const FOnAttributeChangeData& Data);
	void MaxHealthChanged(const FOnAttributeChangeData& Data);

	float CurrentHealth = 0.0f;
	float MaxHealth = 0.0f;
	
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> BoundASC;

	FDelegateHandle HealthChangedHandle;
	FDelegateHandle MaxHealthChangedHandle;
};
