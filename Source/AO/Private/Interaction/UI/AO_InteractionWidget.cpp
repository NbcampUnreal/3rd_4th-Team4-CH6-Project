// HSJ :  AO_InteractionWidget.cpp
#include "Interaction/UI/AO_InteractionWidget.h"

void UAO_InteractionWidget::SetWidgetController(UAO_InteractionWidgetController* InController)
{
	WidgetController = InController;
	OnWidgetControllerSet();
}