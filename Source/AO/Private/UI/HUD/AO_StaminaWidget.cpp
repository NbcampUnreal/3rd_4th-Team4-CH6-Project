// AO_StaminaWidget.cpp

#include "UI/HUD/AO_StaminaWidget.h"

#include "AbilitySystemComponent.h"
#include "Character/AO_PlayerCharacter.h"
#include "Character/GAS/AO_PlayerCharacter_AttributeSet.h"

bool UAO_StaminaWidget::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	if (const TObjectPtr<AAO_PlayerCharacter> PlayerCharacter = Cast<AAO_PlayerCharacter>(GetOwningPlayerPawn()))
	{
		if (TObjectPtr<UAbilitySystemComponent> ASC = PlayerCharacter->GetAbilitySystemComponent())
		{
			BindStaminaDelegates(ASC);
			return true;
		}
	}
	
	return false;
}

void UAO_StaminaWidget::BindStaminaDelegates(UAbilitySystemComponent* ASC)
{
	if (!ASC)
	{
		return;
	}

	if (const UAO_PlayerCharacter_AttributeSet* AttributeSet = ASC->GetSet<UAO_PlayerCharacter_AttributeSet>())
	{
		ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetStaminaAttribute())
			.AddUObject(this, &UAO_StaminaWidget::StaminaChanged);
		ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetMaxStaminaAttribute())
			.AddUObject(this, &UAO_StaminaWidget::MaxStaminaChanged);

		CurrentStamina = AttributeSet->GetStamina();
		MaxStamina = AttributeSet->GetMaxStamina();
		OnStaminaChanged(CurrentStamina, MaxStamina);
	}
}

void UAO_StaminaWidget::StaminaChanged(const FOnAttributeChangeData& Data)
{
	CurrentStamina = Data.NewValue;
	OnStaminaChanged(CurrentStamina, MaxStamina);
}

void UAO_StaminaWidget::MaxStaminaChanged(const FOnAttributeChangeData& Data)
{
	MaxStamina = Data.NewValue;
	OnStaminaChanged(CurrentStamina, MaxStamina);
}
