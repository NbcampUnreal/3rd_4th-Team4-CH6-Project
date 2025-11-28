// AO_PlayerController_Stage.h (장주만)

#pragma once

#include "CoreMinimal.h"
#include "AO_PlayerController_InGameBase.h"
#include "AO_PlayerController_Stage.generated.h"

/**
 * 
 */
UCLASS()
class AO_API AAO_PlayerController_Stage : public AAO_PlayerController_InGameBase
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|UI")
	TSubclassOf<UUserWidget> HUDWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> HUDWidget;
};
