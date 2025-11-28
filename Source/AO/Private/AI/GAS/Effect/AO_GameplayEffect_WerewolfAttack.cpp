// KSJ : AO_GameplayEffect_WerewolfAttack.cpp

#include "AI/GAS/Effect/AO_GameplayEffect_WerewolfAttack.h"
#include "Character/GAS/AO_PlayerCharacter_AttributeSet.h"

UAO_GameplayEffect_WerewolfAttack::UAO_GameplayEffect_WerewolfAttack()
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
