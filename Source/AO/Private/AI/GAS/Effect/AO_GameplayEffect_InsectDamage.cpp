// KSJ : AO_GameplayEffect_InsectDamage.cpp

#include "AI/GAS/Effect/AO_GameplayEffect_InsectDamage.h"
#include "Character/GAS/AO_PlayerCharacter_AttributeSet.h"

UAO_GameplayEffect_InsectDamage::UAO_GameplayEffect_InsectDamage()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;
	
	FGameplayModifierInfo ModInfo;
	ModInfo.Attribute = UAO_PlayerCharacter_AttributeSet::GetHealthAttribute();
	ModInfo.ModifierOp = EGameplayModOp::Additive;
	
	FSetByCallerFloat SetByCaller;
	SetByCaller.DataTag = FGameplayTag::RequestGameplayTag(FName("Data.Damage"));
	ModInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);
	
	Modifiers.Add(ModInfo);
}
