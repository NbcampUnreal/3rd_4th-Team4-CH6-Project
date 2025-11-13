// /HSJ : AO_GameplayAbility_Interact_Info.cpp
#include "Interaction/GAS/Ability/AO_GameplayAbility_Interact_Info.h"
#include "Interaction/Interface/AO_InteractionQuery.h"
#include "Interaction/Interface/AO_Interface_Interactable.h"

bool UAO_GameplayAbility_Interact_Info::InitializeAbility(AActor* TargetActor)
{
	TScriptInterface<IAO_Interface_Interactable> TargetInteractable(TargetActor);
	if (!TargetInteractable)
	{
		return false;
	}

	FAO_InteractionQuery InteractionQuery;
	InteractionQuery.RequestingAvatar = GetAvatarActorFromActorInfo();
	InteractionQuery.RequestingController = GetControllerFromActorInfo();

	// 상호작용 가능 여부 확인
	if (!TargetInteractable->CanInteraction(InteractionQuery))
	{
		return false;
	}

	Interactable = TargetInteractable;
	InteractableActor = TargetActor;

	// 상호작용 정보 수집
	TArray<FAO_InteractionInfo> InteractionInfos;
	FAO_InteractionInfoBuilder InteractionInfoBuilder(Interactable, InteractionInfos);
	Interactable->GatherPostInteractionInfos(InteractionQuery, InteractionInfoBuilder);
	
	if (InteractionInfos.Num() == 0)
	{
		return false;
	}
	
	InteractionInfo = InteractionInfos[0];
	return true;
}
