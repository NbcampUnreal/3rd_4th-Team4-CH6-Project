// JSH: AO_GameInstance.cpp

#include "Game/GameInstance/AO_GameInstance.h"
#include "Game/AO_MapRoutes.h"
#include "AO_Log.h"
#include "GameFramework/PlayerState.h"
#include "OnlineSubsystemTypes.h"
#include "Item/PassiveContainer/AO_Passive_WorldSubsystem.h"
#include "Train/Data/AO_FuelData.h"
#include "Game/GameMode/AO_GameMode_Stage.h"

UAO_GameInstance::UAO_GameInstance()
{
	CurrentStageIndex = 0;
	LobbyHostNetIdStr = TEXT("");
	TeamDeathCount = 0;

	// 최초 기본 부활 횟수
	InitialSharedReviveCount = 2;
	SharedReviveCount = InitialSharedReviveCount;

	SharedTrainFuel = 43.0f; //에셋로드 실패 대비
}

void UAO_GameInstance::Init()
{
	Super::Init();
    
	// 게임 시작 시 BP에서 할당한 에셋으로부터 연료 값을 가져옵니다.
	SharedTrainFuel = GetInitialFuel();
}

void UAO_GameInstance::ResetRun()
{
	CurrentStageIndex = 0;
	SharedTrainFuel = GetInitialFuelValue();
	SharedReviveCount = InitialSharedReviveCount;
	TeamDeathCount = 0;		// JM : 게임 초기화 시 팀 데스 수 초기화
	GameStartTime = 0;
	GameEndTime = 0;
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

//ms : 패시브 초기화
void UAO_GameInstance::PassiveReset()
{
	UAO_Passive_WorldSubsystem* PassiveSub = GetSubsystem<UAO_Passive_WorldSubsystem>();
	if (PassiveSub)
	{
		PassiveSub->ClearAllPlayerData();
	}	
}

float UAO_GameInstance::GetInitialFuel()
{
	if (FuelDataAsset)
	{
		return FuelDataAsset->InitialFuel;
	}
	return SharedTrainFuel;
}

float UAO_GameInstance::GetInitialFuelValue()
{
	if (FuelDataAsset)
	{
		return FuelDataAsset->InitialFuel;
	}
	return 43.0f;
}
// ms
