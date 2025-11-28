// AO_PlayerController_Stage.cpp (장주만)


#include "Player/PlayerController/AO_PlayerController_Stage.h"
#include "AO_Log.h"

AAO_PlayerController_Stage::AAO_PlayerController_Stage()
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_PlayerController_Stage::BeginPlay()
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	Super::BeginPlay();

	if (IsLocalPlayerController())
	{
		if (HUDWidgetClass)
		{
			HUDWidget = CreateWidget<UUserWidget>(this, HUDWidgetClass);
			HUDWidget->AddToViewport();
		}
	}

	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_PlayerController_Stage::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	Super::EndPlay(EndPlayReason);
	AO_LOG(LogJM, Log, TEXT("End"));	
}
