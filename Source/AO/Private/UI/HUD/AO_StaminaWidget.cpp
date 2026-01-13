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
			BindToASC(ASC);
			return true;
		}
	}
	
	return false;
}

void UAO_StaminaWidget::BindToASC(UAbilitySystemComponent* InASC)
{
	if (BoundASC == InASC)
	{
		return;
	}

	UnbindStaminaDelegates();

	BoundASC = InASC;
	if (BoundASC)
	{
		BindStaminaDelegates(BoundASC);
	}
}

void UAO_StaminaWidget::BindToCharacter(ACharacter* InCharacter)
{
	if (!InCharacter)
	{
		BindToASC(nullptr);
		return;
	}

	if (TObjectPtr<UAbilitySystemComponent> ASC = InCharacter->FindComponentByClass<UAbilitySystemComponent>())
	{
		BindToASC(ASC);
	}
	else
	{
		BindToASC(nullptr);
	}
}

void UAO_StaminaWidget::BindStaminaDelegates(UAbilitySystemComponent* ASC)
{
	if (!ASC)
	{
		return;
	}

	if (const UAO_PlayerCharacter_AttributeSet* AttributeSet = ASC->GetSet<UAO_PlayerCharacter_AttributeSet>())
	{
		StaminaChangedHandle = 
			ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetStaminaAttribute())
				.AddUObject(this, &UAO_StaminaWidget::StaminaChanged);
		MaxStaminaChangedHandle = 
			ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetMaxStaminaAttribute())
				.AddUObject(this, &UAO_StaminaWidget::MaxStaminaChanged);

		CurrentStamina = AttributeSet->GetStamina();
		MaxStamina = AttributeSet->GetMaxStamina();
		OnStaminaChanged(CurrentStamina, MaxStamina);
	}
}

void UAO_StaminaWidget::UnbindStaminaDelegates()
{
	if (!BoundASC)
	{
		return;
	}

	if (StaminaChangedHandle.IsValid())
	{
		BoundASC->GetGameplayAttributeValueChangeDelegate(
			UAO_PlayerCharacter_AttributeSet::GetStaminaAttribute())
			.Remove(StaminaChangedHandle);
		StaminaChangedHandle.Reset();
	}

	if (MaxStaminaChangedHandle.IsValid())
	{
		BoundASC->GetGameplayAttributeValueChangeDelegate(
			UAO_PlayerCharacter_AttributeSet::GetMaxStaminaAttribute())
			.Remove(MaxStaminaChangedHandle);
		MaxStaminaChangedHandle.Reset();
	}

	BoundASC = nullptr;
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
