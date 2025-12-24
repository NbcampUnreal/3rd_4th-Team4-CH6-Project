// HSJ : AO_PasswordPanelInspectable.cpp
#include "Puzzle/Element/AO_PasswordPanelInspectable.h"
#include "AO_Log.h"

AAO_PasswordPanelInspectable::AAO_PasswordPanelInspectable()
{
	InspectionTitle = FText::FromString("Password Panel");
	InspectionContent = FText::FromString(": Press F to Inspect");
}

void AAO_PasswordPanelInspectable::OnInspectionMeshClicked(UPrimitiveComponent* ClickedComponent)
{
	if (!ClickedComponent || !HasAuthority())
	{
		return;
	}
	
	MulticastNotifyButtonClicked(ClickedComponent);
}

void AAO_PasswordPanelInspectable::MulticastNotifyButtonClicked_Implementation(UPrimitiveComponent* ButtonComponent)
{
	OnButtonClickedEvent(ButtonComponent);
}