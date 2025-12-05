// HSJ : AO_GameplayAbility_Interact.cpp
#include "Interaction/GAS/Ability/AO_GameplayAbility_Interact.h"
#include "AbilitySystemComponent.h"
#include "Interaction/GAS/Ability/AO_InteractionGameplayAbility.h"
#include "Interaction/GAS/Tag/AO_InteractionGameplayTags.h"
#include "Interaction/GAS/Task/AO_AbilityTask_GrantNearbyInteraction.h"

UAO_GameplayAbility_Interact::UAO_GameplayAbility_Interact()
{
	ActivationPolicy = ECYAbilityActivationPolicy::OnSpawn;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		FGameplayTagContainer Tags;
		Tags.AddTag(AO_InteractionTags::Ability_Action_AbilityInteract);
		SetAssetTags(Tags);
	}
}

void UAO_GameplayAbility_Interact::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 주변 상호작용 오브젝트 탐색 시작(GrantAbilityTask는 주변 상호작용 가능한 오브젝트에게 필요한 어빌리티를 자동으로 부여)
	// 상호작용 오브젝트는 기본적으로 자신에게 어떤 어빌리티가 필요한지 정보를 가지고 있음
	if (TObjectPtr<UAO_AbilityTask_GrantNearbyInteraction> GrantAbilityTask = UAO_AbilityTask_GrantNearbyInteraction::GrantAbilitiesForNearbyInteractables(
		this, InteractionScanRange, InteractionScanRate))
	{
		GrantAbilityTask->ReadyForActivation();
	}
}