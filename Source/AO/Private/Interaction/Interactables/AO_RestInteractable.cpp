// JSH : AO_RestInteractable.cpp

#include "Interaction/Interactables/AO_RestInteractable.h"

#include "Player/PlayerController/AO_PlayerController_Rest.h"
#include "AO/AO_Log.h"

AAO_RestInteractable::AAO_RestInteractable(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InteractType = EAO_RestInteractType::ProceedToNextStage;

	InteractionDuration = 0.0f;
	InteractionTitle = FText::FromString(TEXT("다음 스테이지로 진행"));
	InteractionContent = FText::FromString(TEXT("Press F"));
}

bool AAO_RestInteractable::CanInteraction(const FAO_InteractionQuery& InteractionQuery) const
{
	if(!Super::CanInteraction(InteractionQuery))
	{
		return false;
	}

	// TODO: 필요하다면 여기서 추가 제한 (예: 트래블 중이면 막기 등)

	return true;
}

void AAO_RestInteractable::OnInteractionSuccess_BP_Implementation(AActor* Interactor)
{
	Super::OnInteractionSuccess_BP_Implementation(Interactor);

	APawn* Pawn = Cast<APawn>(Interactor);
	if(Pawn == nullptr)
	{
		return;
	}

	AAO_PlayerController_Rest* PC = Cast<AAO_PlayerController_Rest>(Pawn->GetController());
	if(PC == nullptr)
	{
		return;
	}

	switch(InteractType)
	{
	case EAO_RestInteractType::ProceedToNextStage:
		{
			PC->Server_RequestRestExit();
			break;
		}
	default:
		{
			AO_LOG(LogJSH, Warning,
				TEXT("[RestInteract] Unknown InteractType=%d | This=%s"),
				static_cast<int32>(InteractType),
				*GetName());
			break;
		}
	}
}