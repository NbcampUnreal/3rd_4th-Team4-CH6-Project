// HSJ : AO_GameplayAbility_Interact_Trace.cpp
#include "Interaction/GAS/Ability/AO_GameplayAbility_Interact_Trace.h"

#include "AO_Log.h"
#include "Interaction/Component/AO_InteractionComponent.h"
#include "Interaction/GAS/Task/AO_AbilityTask_WaitForInteractableTraceHit.h"
#include "Interaction/UI/AO_InteractionWidgetController.h"
#include "Physics/AO_CollisionChannels.h"

UAO_GameplayAbility_Interact_Trace::UAO_GameplayAbility_Interact_Trace()
{
	ActivationPolicy = ECYAbilityActivationPolicy::OnSpawn;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;
}

void UAO_GameplayAbility_Interact_Trace::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	FAO_InteractionQuery InteractionQuery;
	InteractionQuery.RequestingAvatar = GetAvatarActorFromActorInfo();
	InteractionQuery.RequestingController = GetControllerFromActorInfo();

	if (TObjectPtr<UAO_AbilityTask_WaitForInteractableTraceHit> TraceHitTask = UAO_AbilityTask_WaitForInteractableTraceHit::WaitForInteractableTraceHit(
		this, InteractionQuery, AO_TraceChannel_Interaction, 
		MakeTargetLocationInfoFromOwnerActor(), 
		InteractionTraceRange, InteractionTraceRate, bShowTraceDebug, TraceSphereRadius))
	{
		TraceHitTask->InteractableChanged.AddDynamic(this, &ThisClass::UpdateInteractions);
		TraceHitTask->ReadyForActivation();
	}
	else
	{
		AO_LOG(LogHSJ, Error, TEXT("Failed to create trace task"));
	}
}

void UAO_GameplayAbility_Interact_Trace::UpdateInteractions(const TArray<FAO_InteractionInfo>& InteractionInfos)
{
	TObjectPtr<AActor> AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor)
	{
		AO_LOG(LogHSJ, Error, TEXT("AvatarActor is null"));
		return;
	}

	TObjectPtr<UAO_InteractionComponent> InteractionComp = AvatarActor->FindComponentByClass<UAO_InteractionComponent>();
	if (!InteractionComp)
	{
		AO_LOG(LogHSJ, Error, TEXT("InteractionComponent not found"));
		return;
	}
	
	if (TObjectPtr<UAO_InteractionWidgetController> Controller = InteractionComp->GetInteractionWidgetController())
	{
		FAO_InteractionMessage Message;
		Message.MessageType = EAO_InteractionMessageType::Notice;
		Message.Instigator = AvatarActor;
		Message.bShouldRefresh = true;
		Message.InteractionInfo = InteractionInfos.Num() > 0 ? InteractionInfos[0] : FAO_InteractionInfo();
		
		Controller->BroadcastInteractionMessage(Message);
	}
	
	// 타겟 액터 추출 및 설정
	TObjectPtr<AActor> TargetActor = nullptr;
	if (InteractionInfos.Num() > 0)
	{
		if (TObjectPtr<UObject> Object = InteractionInfos[0].Interactable.GetObject())
		{
			if (TObjectPtr<AActor> Actor = Cast<AActor>(Object))
			{
				TargetActor = Actor;
			}
			else if (TObjectPtr<UActorComponent> Component = Cast<UActorComponent>(Object))
			{
				TargetActor = Component->GetOwner();
			}
		}
	}
	
	InteractionComp->SetCurrentInteractTarget(TargetActor);
}