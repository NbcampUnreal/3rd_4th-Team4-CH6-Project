// AO_SpectateWidget.h

#pragma once

#include "CoreMinimal.h"
#include "AO_UserWidget.h"
#include "AO_SpectateWidget.generated.h"

UCLASS()
class AO_API UAO_SpectateWidget : public UAO_UserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetSpectatingPlayerName(const FText& InName);
};
