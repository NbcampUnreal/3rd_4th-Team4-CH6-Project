// HSJ : GA_Interact_Base.cpp
#include "Interaction/Base/GA_Interact_Base.h"
#include "Interaction/Actor/AO_WorldInteractable.h"

UGA_Interact_Base::UGA_Interact_Base()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
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

	// WorldInteractable 체크
	AAO_WorldInteractable* Interactable = Cast<AAO_WorldInteractable>(TargetActor);
	if (!Interactable)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// OnInteractionSuccess 호출
	Interactable->OnInteractionSuccess(Instigator);

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}