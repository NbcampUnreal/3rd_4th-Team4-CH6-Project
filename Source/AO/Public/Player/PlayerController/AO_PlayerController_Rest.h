// JSH : AO_PlayerController_Rest.h

#pragma once

#include "CoreMinimal.h"
#include "Player/PlayerController/AO_PlayerController_InGameBase.h"
#include "AO_PlayerController_Rest.generated.h"

UCLASS()
class AO_API AAO_PlayerController_Rest : public AAO_PlayerController_InGameBase
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	
public:
	UFUNCTION(Server, Reliable)
	void Server_RequestRestExit();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|UI")
	TSubclassOf<UUserWidget> HUDWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> HUDWidget;
};
