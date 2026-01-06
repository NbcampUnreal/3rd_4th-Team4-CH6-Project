
#pragma once

#include "CoreMinimal.h"
#include "InputModifiers.h"
#include "AO_InputModifier.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, meta = (DisplayName="Slot Index Modifier"))
class AO_API UInputModifier_SlotIndex : public UInputModifier
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category="Slot")
	int32 SlotIndex = 0;

	virtual FInputActionValue ModifyRaw_Implementation(
	   const UEnhancedPlayerInput* PlayerInput,
	   FInputActionValue CurrentValue,
	   float DeltaTime
	) override
	{
		return FInputActionValue((float)SlotIndex); 
	}
};