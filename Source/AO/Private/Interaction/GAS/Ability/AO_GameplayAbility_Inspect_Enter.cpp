// HSJ : AO_GameplayAbility_Inspect_Enter.cpp
#include "Interaction/GAS/Ability/AO_GameplayAbility_Inspect_Enter.h"
#include "Interaction/Component/AO_InspectionComponent.h"
#include "Interaction/Component/AO_InspectableComponent.h"
#include "Interaction/GAS/Tag/AO_InteractionGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "AO_Log.h"

UAO_GameplayAbility_Inspect_Enter::UAO_GameplayAbility_Inspect_Enter()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

    ActivationOwnedTags.AddTag(AO_InteractionTags::Status_Action_Inspecting);
    BlockAbilitiesWithTag.AddTag(AO_InteractionTags::Ability_Action_AbilityInteract);
}

void UAO_GameplayAbility_Inspect_Enter::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    check(ActorInfo->OwnerActor->HasAuthority());
    
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    if (!TriggerEventData || !TriggerEventData->Target.Get())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    TObjectPtr<AActor> TargetActor = const_cast<AActor*>(TriggerEventData->Target.Get());
    TObjectPtr<AActor> Instigator = const_cast<AActor*>(TriggerEventData->Instigator.Get());

    TObjectPtr<UAO_InspectableComponent> InspectableComp = TargetActor->FindComponentByClass<UAO_InspectableComponent>();
    if (!InspectableComp)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

	// 다른 사람이 이미 조사중일 경우
    if (InspectableComp->IsLockedByOtherPlayer(Instigator))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    TObjectPtr<UAO_InspectionComponent> InspectionComp = Instigator->FindComponentByClass<UAO_InspectionComponent>();
    if (!InspectionComp)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    InspectableComp->SetInspectionLocked(true, Instigator);
    
    InspectionComp->SetInspectEnterHandle(Handle);
    InspectionComp->EnterInspectionMode(TargetActor);
}

void UAO_GameplayAbility_Inspect_Enter::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}