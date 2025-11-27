// JSH: AO_GameMode_Rest.cpp

#include "Game/GameMode/AO_GameMode_Rest.h"
#include "Game/GameInstance/AO_GameInstance.h"
#include "Player/PlayerController/AO_PlayerController_Stage.h"
#include "AO_Log.h"


AAO_GameMode_Rest::AAO_GameMode_Rest()
{
	PlayerControllerClass = AAO_PlayerController_Stage::StaticClass();

	AO_LOG(LogJSH, Log, TEXT("RestGameMode: Constructor"));
}

void AAO_GameMode_Rest::BeginPlay()
{
	Super::BeginPlay();

	AO_LOG(LogJSH, Log, TEXT("RestGameMode: BeginPlay"));

	if(UGameInstance* GI = GetGameInstance())
	{
		if(UAO_GameInstance* AO_GI = Cast<UAO_GameInstance>(GI))
		{
			AO_LOG(LogJSH, Log, TEXT("Stage BeginPlay: GI Fuel = %.1f"), AO_GI->SharedTrainFuel);
		}
	}
}
