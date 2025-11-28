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

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "GAS|UI")
	void OnHealthChanged(float NewHealth, float NewMaxHealth);

private:
	void BindHealthDelegates(UAbilitySystemComponent* ASC);
	
	void HealthChanged(const FOnAttributeChangeData& Data);
	void MaxHealthChanged(const FOnAttributeChangeData& Data);

	float CurrentHealth = 0.0f;
	float MaxHealth = 0.0f;
};
