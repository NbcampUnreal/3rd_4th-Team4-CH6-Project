// AO_PlayerCharacter_AttributeSet.cpp

#include "Character/GAS/AO_PlayerCharacter_AttributeSet.h"

#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

UAO_PlayerCharacter_AttributeSet::UAO_PlayerCharacter_AttributeSet()
{
	InitHealth(100.f);
	InitMaxHealth(100.f);
	InitStamina(100.f);
	InitMaxStamina(100.f);
	InitWalkSpeed(200.f);
	InitRunSpeed(500.f);
	InitSprintSpeed(800.f);
}

void UAO_PlayerCharacter_AttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UAO_PlayerCharacter_AttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAO_PlayerCharacter_AttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAO_PlayerCharacter_AttributeSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAO_PlayerCharacter_AttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAO_PlayerCharacter_AttributeSet, WalkSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAO_PlayerCharacter_AttributeSet, RunSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAO_PlayerCharacter_AttributeSet, SprintSpeed, COND_None, REPNOTIFY_Always);
}

void UAO_PlayerCharacter_AttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	else if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxStamina());
	}
}

void UAO_PlayerCharacter_AttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		const float NewHealth = GetHealth();
		
		if (NewHealth <= 0.f)
		{
			if (AActor* Owner = GetOwningActor())
			{
				if (Owner->HasAuthority())
				{
					OnPlayerDeath.Broadcast();
				}
			}
		}
		
		SetHealth(FMath::Clamp(NewHealth, 0.f, GetMaxHealth()));
	}
	else if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		SetStamina(FMath::Clamp(GetStamina(), 0.f, GetMaxStamina()));

		HandleStaminaLockout(Data);
	}
}

void UAO_PlayerCharacter_AttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAO_PlayerCharacter_AttributeSet, Health, OldValue);
}

void UAO_PlayerCharacter_AttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAO_PlayerCharacter_AttributeSet, MaxHealth, OldValue);
}

void UAO_PlayerCharacter_AttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAO_PlayerCharacter_AttributeSet, Stamina, OldValue);
}

void UAO_PlayerCharacter_AttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAO_PlayerCharacter_AttributeSet, MaxStamina, OldValue);
}

void UAO_PlayerCharacter_AttributeSet::OnRep_WalkSpeed(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAO_PlayerCharacter_AttributeSet, WalkSpeed, OldValue);
}

void UAO_PlayerCharacter_AttributeSet::OnRep_RunSpeed(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAO_PlayerCharacter_AttributeSet, RunSpeed, OldValue);
}

void UAO_PlayerCharacter_AttributeSet::OnRep_SprintSpeed(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAO_PlayerCharacter_AttributeSet, SprintSpeed, OldValue);
}

void UAO_PlayerCharacter_AttributeSet::HandleStaminaLockout(const struct FGameplayEffectModCallbackData& Data)
{
	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponentChecked();

	AActor* OwningActor = GetOwningActor();
	if (!OwningActor || !OwningActor->HasAuthority())
	{
		return;
	}

	const float NewStamina = GetStamina();

	const float Threshold = GetMaxStamina() * StaminaLockoutPercent;
	const FGameplayTag LockoutTag = FGameplayTag::RequestGameplayTag(FName("Status.Lockout.Stamina"));
	
	if (NewStamina <= 0.f)
	{
		if (!ASC->HasMatchingGameplayTag(LockoutTag))
		{
			ASC->AddLooseGameplayTag(LockoutTag);
			
			FGameplayTagContainer SprintTag;
			SprintTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Movement.Sprint")));
			ASC->CancelAbilities(&SprintTag);
		}
	}
	else if (ASC->HasMatchingGameplayTag(LockoutTag) && NewStamina >= Threshold)
	{
		ASC->RemoveLooseGameplayTag(LockoutTag);
	}
}
