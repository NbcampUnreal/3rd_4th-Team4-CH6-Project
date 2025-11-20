// HSJ : GA_Interact_Base.cpp
#include "Interaction/Base/GA_Interact_Base.h"
#include "Interaction/Actor/AO_WorldInteractable.h"
#include "Interaction/Component/AO_InteractableComponent.h"
#include "Interaction/GAS/Tag/AO_InteractionGameplayTags.h"

UGA_Interact_Base::UGA_Interact_Base()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		FAbilityTriggerData TriggerData;
		TriggerData.TriggerTag = AO_InteractionTags::Ability_Action_AbilityInteract_Finalize;
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(TriggerData);
	}
}

void UGA_Interact_Base::ActivateAbility(
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

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!TriggerEventData || !TriggerEventData->Target.Get())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AActor* TargetActor = const_cast<AActor*>(TriggerEventData->Target.Get());
	AActor* Instigator = const_cast<AActor*>(TriggerEventData->Instigator.Get());
	
	// WorldInteractable 체크(상속일 경우)
	if (AAO_WorldInteractable* Interactable = Cast<AAO_WorldInteractable>(TargetActor))
	{
		Interactable->OnInteractionSuccess(Instigator);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	
	// InteractableComponent 체크(컴포넌트일 경우)
	if (UAO_InteractableComponent* InteractableComp = TargetActor->FindComponentByClass<UAO_InteractableComponent>())
	{
		InteractableComp->NotifyInteractionSuccess(Instigator);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
}