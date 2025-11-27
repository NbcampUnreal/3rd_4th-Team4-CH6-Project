// AO_GameMode_Stage.cpp (장주만)


#include "Game/GameMode/AO_GameMode_Stage.h"
#include "Game/GameInstance/AO_GameInstance.h"
#include "AO_Log.h"

AAO_GameMode_Stage::AAO_GameMode_Stage()
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_GameMode_Stage::HandleStageExitRequest(AController* Requester)
{
	if(!HasAuthority())
	{
		return;
	}

	UGameInstance* GI = GetGameInstance();
	UAO_GameInstance* AO_GI = GI ? Cast<UAO_GameInstance>(GI) : nullptr;

	if(!AO_GI)
	{
		AO_LOG(LogJSH, Warning, TEXT("StageExit: GameInstance is not UAO_GameInstance"));
		return;
	}

	const float Fuel = AO_GI->SharedTrainFuel;

	if(Fuel < 20.0f)
	{
		AO_LOG(LogJSH, Warning, TEXT("StageExit: Not enough fuel (%.1f / 20.0)"), Fuel);
		return;
	}

	AO_LOG(LogJSH, Log, TEXT("StageExit: OK, Fuel=%.1f → Travel to next map"), Fuel);

	if(UWorld* World = GetWorld())
	{
		const FString NextMap = TEXT("/Game/AVaOut/Maps/LV_RestRoom?listen");
		World->ServerTravel(NextMap);
	}
}