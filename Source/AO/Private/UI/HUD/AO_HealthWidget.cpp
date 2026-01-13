// AO_HealthWidget.cpp

#include "UI/HUD/AO_HealthWidget.h"

#include "AbilitySystemComponent.h"
#include "Character/AO_PlayerCharacter.h"
#include "Character/GAS/AO_PlayerCharacter_AttributeSet.h"

bool UAO_HealthWidget::Initialize()
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

void UAO_HealthWidget::BindToASC(UAbilitySystemComponent* InASC)
{
	if (BoundASC == InASC)
	{
		return;
	}

	UnbindHealthDelegates();

	BoundASC = InASC;
	if (BoundASC)
	{
		BindHealthDelegates(BoundASC);
	}
}

void UAO_HealthWidget::BindToCharacter(ACharacter* InCharacter)
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

void UAO_HealthWidget::BindHealthDelegates(UAbilitySystemComponent* ASC)
{
	if (!ASC)
	{
		return;
	}

	if (const UAO_PlayerCharacter_AttributeSet* AttributeSet = ASC->GetSet<UAO_PlayerCharacter_AttributeSet>())
	{
		HealthChangedHandle = 
			ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute())
				.AddUObject(this, &UAO_HealthWidget::HealthChanged);
		MaxHealthChangedHandle = 
			ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetMaxHealthAttribute())
				.AddUObject(this, &UAO_HealthWidget::MaxHealthChanged);

		CurrentHealth = AttributeSet->GetHealth();
		MaxHealth = AttributeSet->GetMaxHealth();
		OnHealthChanged(CurrentHealth, MaxHealth);
	}
}

void UAO_HealthWidget::UnbindHealthDelegates()
{
	if (!BoundASC)
	{
		return;
	}

	if (HealthChangedHandle.IsValid())
	{
		BoundASC->GetGameplayAttributeValueChangeDelegate(
			UAO_PlayerCharacter_AttributeSet::GetHealthAttribute())
			.Remove(HealthChangedHandle);
		HealthChangedHandle.Reset();
	}

	if (MaxHealthChangedHandle.IsValid())
	{
		BoundASC->GetGameplayAttributeValueChangeDelegate(
			UAO_PlayerCharacter_AttributeSet::GetMaxHealthAttribute())
			.Remove(MaxHealthChangedHandle);
		MaxHealthChangedHandle.Reset();
	}

	BoundASC = nullptr;
}

void UAO_HealthWidget::HealthChanged(const FOnAttributeChangeData& Data)
{
	CurrentHealth = Data.NewValue;
	OnHealthChanged(CurrentHealth, MaxHealth);
}

void UAO_HealthWidget::MaxHealthChanged(const FOnAttributeChangeData& Data)
{
	MaxHealth = Data.NewValue;
	OnHealthChanged(CurrentHealth, MaxHealth);
}
