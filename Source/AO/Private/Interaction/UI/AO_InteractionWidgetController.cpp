// HSJ :  AO_InteractionWidgetController.cpp
#include "Interaction/UI/AO_InteractionWidgetController.h"

void UAO_InteractionWidgetController::BroadcastInteractionMessage(FAO_InteractionMessage Message)
{
	OnInteractionMessageReceived.Broadcast(Message);
}

void UAO_InteractionWidgetController::HideInteraction()
{
	FAO_InteractionMessage EmptyMessage;
	OnInteractionMessageReceived.Broadcast(EmptyMessage);
}