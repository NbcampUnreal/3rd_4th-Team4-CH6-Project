#include "Train/GAS/AO_Fuel_AttributeSet.h"
#include "Net/UnrealNetwork.h"

UAO_Fuel_AttributeSet::UAO_Fuel_AttributeSet()
{
	Fuel.SetBaseValue(0.f);
	Fuel.SetCurrentValue(0.f);
}

void UAO_Fuel_AttributeSet::OnRep_Fuel(const FGameplayAttributeData& OldFuel)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAO_Fuel_AttributeSet, Fuel, OldFuel);

	UE_LOG(LogTemp, Warning, TEXT("🔥 Fuel changed: %.1f → %.1f"), OldFuel.GetCurrentValue(), Fuel.GetCurrentValue());
}

void UAO_Fuel_AttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UAO_Fuel_AttributeSet, Fuel, COND_None, REPNOTIFY_Always);
}
