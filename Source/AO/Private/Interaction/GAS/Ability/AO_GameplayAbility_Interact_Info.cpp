// /HSJ : AO_GameplayAbility_Interact_Info.cpp
#include "Interaction/GAS/Ability/AO_GameplayAbility_Interact_Info.h"
#include "Interaction/Component/AO_InteractableComponent.h"
#include "Interaction/Interface/AO_InteractionQuery.h"
#include "Interaction/Interface/AO_Interface_Interactable.h"

bool UAO_GameplayAbility_Interact_Info::InitializeAbility(AActor* TargetActor)
{
	TScriptInterface<IAO_Interface_Interactable> TargetInteractable;

	// Actor 자체가 인터페이스 구현하는지 체크
	TargetInteractable = TScriptInterface<IAO_Interface_Interactable>(TargetActor);
    
	// 그렇지 않으면 InteractableComponent 컴포넌트가 있는지 찾기
	if (!TargetInteractable)
	{
		if (TObjectPtr<UAO_InteractableComponent> InteractableComp = TargetActor->FindComponentByClass<UAO_InteractableComponent>())
		{
			TargetInteractable = TScriptInterface<IAO_Interface_Interactable>(InteractableComp);
		}
	}
	
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
	InteractionInfo = TargetInteractable->GetInteractionInfo(InteractionQuery);
	InteractionInfo.Interactable = TargetInteractable;
	
	// Duration 보정 (음수 방지)
	InteractionInfo.Duration = FMath::Max(0.f, InteractionInfo.Duration);
	
	this->InteractionInfo = InteractionInfo;
	return true;
}
