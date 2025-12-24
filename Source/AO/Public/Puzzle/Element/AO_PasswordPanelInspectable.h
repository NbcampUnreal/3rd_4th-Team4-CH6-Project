// HSJ : AO_PasswordPanelInspectable.h
#pragma once

#include "CoreMinimal.h"
#include "Puzzle/Element/AO_InspectionPuzzle.h"
#include "AO_PasswordPanelInspectable.generated.h"

UCLASS()
class AO_API AAO_PasswordPanelInspectable : public AAO_InspectionPuzzle
{
	GENERATED_BODY()

public:
	AAO_PasswordPanelInspectable();

	virtual void OnInspectionMeshClicked(UPrimitiveComponent* ClickedComponent) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Password")
	void OnButtonClickedEvent(UPrimitiveComponent* ButtonComponent);

protected:
	UFUNCTION(NetMulticast, Reliable)
	void MulticastNotifyButtonClicked(UPrimitiveComponent* ButtonComponent);
};