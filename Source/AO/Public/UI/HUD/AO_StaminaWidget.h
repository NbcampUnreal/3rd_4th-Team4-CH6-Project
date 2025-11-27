// AO_StaminaWidget.h

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/AO_UserWidget.h"
#include "AO_StaminaWidget.generated.h"

struct FOnAttributeChangeData;
class UAbilitySystemComponent;

UCLASS()
class AO_API UAO_StaminaWidget : public UAO_UserWidget
{
	GENERATED_BODY()

public:
	virtual bool Initialize() override;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "GAS|UI")
	void OnStaminaChanged(float NewStamina, float NewMaxStamina);

private:
	void BindStaminaDelegates(UAbilitySystemComponent* ASC);

	void StaminaChanged(const FOnAttributeChangeData& Data);
	void MaxStaminaChanged(const FOnAttributeChangeData& Data);

	float CurrentStamina = 0.0f;
	float MaxStamina = 0.0f;
};
