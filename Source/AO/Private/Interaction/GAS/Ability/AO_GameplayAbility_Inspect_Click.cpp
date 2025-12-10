// HSJ : AO_GameplayAbility_Inspect_Click.cpp
#include "Interaction/GAS/Ability/AO_GameplayAbility_Inspect_Click.h"
#include "Interaction/Component/AO_InspectionComponent.h"
#include "Interaction/Interface/AO_Interface_Inspectable.h"
#include "Interaction/GAS/Tag/AO_InteractionGameplayTags.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "AO_Log.h"

UAO_GameplayAbility_Inspect_Click::UAO_GameplayAbility_Inspect_Click()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

    // Inspection 중일 때만 활성화 가능
    ActivationRequiredTags.AddTag(AO_InteractionTags::Status_Action_Inspecting);
}

void UAO_GameplayAbility_Inspect_Click::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    TObjectPtr<AActor> AvatarActor = GetAvatarActorFromActorInfo();
    if (!AvatarActor)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

	// InspectionComponent에서 현재 검사 중인 액터 확인
	TObjectPtr<UAO_InspectionComponent> InspectionComp = AvatarActor->FindComponentByClass<UAO_InspectionComponent>();
	if (!InspectionComp || !InspectionComp->GetInspectedActor())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (InspectionComp->CachedHoverComponent.IsValid() && 
		InspectionComp->CachedHoverActor.IsValid())
	{
		InspectionComp->ServerProcessInspectionClick(
			InspectionComp->CachedHoverActor.Get(), 
			InspectionComp->CachedHoverComponent->GetFName()
		);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
}