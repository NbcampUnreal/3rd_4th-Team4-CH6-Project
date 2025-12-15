// AO_AIAttributeSet.cpp

#include "AI/GAS/AO_AIAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

UAO_AIAttributeSet::UAO_AIAttributeSet()
{
	InitMovementSpeed(300.f);
	InitMaxMovementSpeed(600.f);
}

void UAO_AIAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UAO_AIAttributeSet, MovementSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAO_AIAttributeSet, MaxMovementSpeed, COND_None, REPNOTIFY_Always);
}

void UAO_AIAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetMovementSpeedAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMovementSpeed());
	}
}

void UAO_AIAttributeSet::OnRep_MovementSpeed(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAO_AIAttributeSet, MovementSpeed, OldValue);
}

void UAO_AIAttributeSet::OnRep_MaxMovementSpeed(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAO_AIAttributeSet, MaxMovementSpeed, OldValue);
}
