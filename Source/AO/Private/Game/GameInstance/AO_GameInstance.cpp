// JSH: AO_GameInstance.cpp

#include "Game/GameInstance/AO_GameInstance.h"
#include "Game/AO_MapRoutes.h"
#include "AO_Log.h"

UAO_GameInstance::UAO_GameInstance()
{
	SharedTrainFuel = 0.0f;
	CurrentStageIndex = 0;
}

void UAO_GameInstance::ResetRun()
{
	CurrentStageIndex = 0;
	SharedTrainFuel = 0.0f;
}

FName UAO_GameInstance::GetCurrentStageMap() const
{
	return AO_MapRoutes::GetStageMapName(CurrentStageIndex);
}

FName UAO_GameInstance::GetRestMap() const
{
	return AO_MapRoutes::GetRestMapName();
}

FName UAO_GameInstance::GetLobbyMap() const
{
	return AO_MapRoutes::GetLobbyMapName();
}

bool UAO_GameInstance::IsLastStage() const
{
	const int32 StageCount = AO_MapRoutes::GetStageCount();

	if(StageCount <= 0)
	{
		return false;
	}

	return CurrentStageIndex == StageCount - 1;
}

bool UAO_GameInstance::TryAdvanceStageIndex()
{
	const int32 StageCount = AO_MapRoutes::GetStageCount();

	if(CurrentStageIndex + 1 < StageCount)
	{
		++CurrentStageIndex;
		return true;
	}

	return false;
}
