// JSH: AO_StageInteractable.cpp 

#include "Interaction/Interactables/AO_StageInteractable.h"
#include "Player/PlayerController/AO_PlayerController_Stage.h"




AAO_StageInteractable::AAO_StageInteractable(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InteractType = EAO_StageInteractType::ExitToNextArea;
	RequiredFuel = 20.0f;
}

bool AAO_StageInteractable::CanInteraction(const FAO_InteractionQuery& InteractionQuery) const
{
	if(!Super::CanInteraction(InteractionQuery))
	{
		return false;
	}

	// TODO: 나중에 여기서 GI 연료를 미리 체크해서
	// "연료가 부족합니다" 같은 UI만 막고 싶다면 조건 추가

	return true;
}

void AAO_StageInteractable::OnInteractionSuccess_BP_Implementation(AActor* Interactor)
{
	Super::OnInteractionSuccess_BP_Implementation(Interactor);

	APawn* Pawn = Cast<APawn>(Interactor);
	if(!Pawn)
	{
		return;
	}
	
	AAO_PlayerController_Stage* PC = Cast<AAO_PlayerController_Stage>(Pawn->GetController());
	if(!PC)
	{
		return;
	}

	switch(InteractType)
	{
	case EAO_StageInteractType::ExitToNextArea:
		PC->Server_RequestStageExit();
		break;

	case EAO_StageInteractType::ExitToLobby:
		// 추후 필요시 구현
		break;

	default:
		break;
	}
}
