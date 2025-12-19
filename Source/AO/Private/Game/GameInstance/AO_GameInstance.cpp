// JSH: AO_GameInstance.cpp

#include "Game/GameInstance/AO_GameInstance.h"
#include "Game/AO_MapRoutes.h"
#include "AO_Log.h"
#include "GameFramework/PlayerState.h"
#include "OnlineSubsystemTypes.h"

UAO_GameInstance::UAO_GameInstance()
{
	SharedTrainFuel = 40.0f;
	CurrentStageIndex = 0;
	LobbyHostNetIdStr = TEXT("");

	// 최초 기본 부활 횟수
	InitialSharedReviveCount = 5;
	SharedReviveCount = InitialSharedReviveCount;
}

void UAO_GameInstance::ResetRun()
{
	CurrentStageIndex = 0;
	SharedTrainFuel = 40.0f;
	SharedReviveCount = InitialSharedReviveCount;
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

// ===== 세션 리셋 =====

void UAO_GameInstance::ResetSessionData()
{
	// 스테이지 인덱스 / 연료 초기화
	ResetRun();

	// 로비 호스트 정보 초기화
	ClearLobbyHostInfo();
}

// ===== 호스트 정보 헬퍼 =====

void UAO_GameInstance::ClearLobbyHostInfo()
{
	LobbyHostNetIdStr = TEXT("");
}

bool UAO_GameInstance::HasLobbyHost() const
{
	return !LobbyHostNetIdStr.IsEmpty();
}

void UAO_GameInstance::SetLobbyHostFromPlayerState(const APlayerState* PlayerState)
{
	if(PlayerState == nullptr)
	{
		return;
	}

	const FUniqueNetIdRepl& IdRepl = PlayerState->GetUniqueId();
	if(IdRepl.IsValid() == false)
	{
		return;
	}

	const TSharedPtr<const FUniqueNetId> NetId = IdRepl.GetUniqueNetId();
	if(NetId.IsValid() == false)
	{
		return;
	}

	LobbyHostNetIdStr = NetId->ToString();
}

bool UAO_GameInstance::IsLobbyHostPlayerState(const APlayerState* PlayerState) const
{
	if(PlayerState == nullptr)
	{
		return false;
	}

	if(LobbyHostNetIdStr.IsEmpty())
	{
		return false;
	}

	const FUniqueNetIdRepl& IdRepl = PlayerState->GetUniqueId();
	if(IdRepl.IsValid() == false)
	{
		return false;
	}

	const TSharedPtr<const FUniqueNetId> NetId = IdRepl.GetUniqueNetId();
	if(NetId.IsValid() == false)
	{
		return false;
	}

	return LobbyHostNetIdStr == NetId->ToString();
}

int32 UAO_GameInstance::GetSharedReviveCount() const
{
	return SharedReviveCount;
}

void UAO_GameInstance::AddSharedReviveCount(int32 Delta)
{
	const int32 NewValue = SharedReviveCount + Delta;

	if (NewValue < 0)
	{
		SharedReviveCount = 0;
	}
	else
	{
		SharedReviveCount = NewValue;
	}

	AO_LOG(LogJSH, Log, TEXT("GI: SharedReviveCount changed to %d"), SharedReviveCount);
}

bool UAO_GameInstance::TryConsumeSharedReviveCount()
{
	if (SharedReviveCount <= 0)
	{
		return false;
	}

	--SharedReviveCount;

	AO_LOG(LogJSH, Log, TEXT("GI: Consume revive -> %d left"), SharedReviveCount);

	return true;
}
