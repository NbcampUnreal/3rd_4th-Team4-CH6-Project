// KSJ : AO_GameplayEffect_TrollAttack.cpp

#include "AI/GAS/Effect/AO_GameplayEffect_TrollAttack.h"
#include "Character/GAS/AO_PlayerCharacter_AttributeSet.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"

UAO_GameplayEffect_TrollAttack::UAO_GameplayEffect_TrollAttack()
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