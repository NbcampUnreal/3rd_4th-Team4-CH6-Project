// AO_PlayerController_Stage.cpp (장주만)


#include "Player/PlayerController/AO_PlayerController_Stage.h"

void AAO_PlayerController_Stage::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalPlayerController())
	{
		if (HUDWidgetClass)
		{
			HUDWidget = CreateWidget<UUserWidget>(this, HUDWidgetClass);
			HUDWidget->AddToViewport();
		}
	}
}
