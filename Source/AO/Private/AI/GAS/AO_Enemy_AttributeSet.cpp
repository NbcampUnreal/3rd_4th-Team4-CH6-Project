// KSJ : AO_Enemy_AttributeSet.cpp

#include "AI/GAS/AO_Enemy_AttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "AO_Log.h"

UAO_Enemy_AttributeSet::UAO_Enemy_AttributeSet()
{
	InitHealth(100.0f);
	InitMaxHealth(100.0f);
	InitMovementSpeed(300.0f);
	InitAttackDamage(10.0f);
	InitAttackRange(150.0f);
}

void UAO_Enemy_AttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION_NOTIFY(UAO_Enemy_AttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAO_Enemy_AttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAO_Enemy_AttributeSet, MovementSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAO_Enemy_AttributeSet, AttackDamage, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAO_Enemy_AttributeSet, AttackRange, COND_None, REPNOTIFY_Always);
}

void UAO_Enemy_AttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAO_Enemy_AttributeSet, Health, OldHealth);
}

void UAO_Enemy_AttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAO_Enemy_AttributeSet, MaxHealth, OldMaxHealth);
}

void UAO_Enemy_AttributeSet::OnRep_MovementSpeed(const FGameplayAttributeData& OldMovementSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAO_Enemy_AttributeSet, MovementSpeed, OldMovementSpeed);
}

void UAO_Enemy_AttributeSet::OnRep_AttackDamage(const FGameplayAttributeData& OldAttackDamage)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAO_Enemy_AttributeSet, AttackDamage, OldAttackDamage);
}

void UAO_Enemy_AttributeSet::OnRep_AttackRange(const FGameplayAttributeData& OldAttackRange)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAO_Enemy_AttributeSet, AttackRange, OldAttackRange);
}

