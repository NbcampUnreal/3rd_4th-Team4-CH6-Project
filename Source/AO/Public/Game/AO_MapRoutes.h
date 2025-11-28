// JSH: AO_MapRoutes.h
#pragma once

#include "CoreMinimal.h"

namespace AO_MapRoutes
{
	// 스테이지 맵 경로들 (필요에 따라 수정)
	static constexpr const TCHAR* STAGE_MAPS[] =
	{
		TEXT("/Game/AVaOut/Train/TrainGASTest"),
		TEXT("/Game/AVaOut/Train/TrainGASTest"),
		TEXT("/Game/AVaOut/Train/TrainGASTest"),
	};

	static constexpr int32 STAGE_MAP_COUNT = UE_ARRAY_COUNT(STAGE_MAPS);

	static constexpr const TCHAR* LOBBY_MAP = TEXT("/Game/AVaOut/Maps/LV_Lobby");
	static constexpr const TCHAR* REST_MAP  = TEXT("/Game/AVaOut/Maps/LV_RestRoom");

	static FName GetStageMapName(int32 StageIndex)
	{
		if(StageIndex < 0 || StageIndex >= STAGE_MAP_COUNT)
		{
			return NAME_None;
		}

		return FName(STAGE_MAPS[StageIndex]);
	}

	static int32 GetStageCount()
	{
		return STAGE_MAP_COUNT;
	}

	static FName GetLobbyMapName()
	{
		return FName(LOBBY_MAP);
	}

	static FName GetRestMapName()
	{
		return FName(REST_MAP);
	}
}
