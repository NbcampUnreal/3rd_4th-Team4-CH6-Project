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

	// 최초 기본 부활 횟수
	InitialSharedReviveCount = 5;
	SharedReviveCount = InitialSharedReviveCount;

	SharedTrainFuel = 43.0f; //에셋로드 실패 대비
}

void UAO_GameInstance::Init()
{
	Super::Init();
    
	// 게임 시작 시 BP에서 할당한 에셋으로부터 연료 값을 가져옵니다.
	SharedTrainFuel = GetInitialFuel();
    
	UE_LOG(LogTemp, Log, TEXT("UAO_GameInstance::Init - Initial Fuel: %f"), SharedTrainFuel);
}

void UAO_GameInstance::ResetRun()
{
	CurrentStageIndex = 0;
	SharedTrainFuel = GetInitialFuelValue();
	SharedReviveCount = InitialSharedReviveCount;
    
	UE_LOG(LogTemp, Log, TEXT("Run Reset: SharedTrainFuel set to %f"), SharedTrainFuel);
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
	const int32 OldValue = SharedReviveCount;
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
	
	// 값이 증가한 경우에만 스테이지 GameMode에 알림
	if (SharedReviveCount > OldValue)
	{
		UWorld* World = GetWorld();
		if (World != nullptr)
		{
			if (AAO_GameMode_Stage* StageGM = World->GetAuthGameMode<AAO_GameMode_Stage>())
			{
				StageGM->HandleSharedReviveCountIncreased();
			}
		}
	}
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

//ms : 패시브 초기화
void UAO_GameInstance::PassiveReset()
{
	UAO_Passive_WorldSubsystem* PassiveSub = GetSubsystem<UAO_Passive_WorldSubsystem>();
	if (PassiveSub)
	{
		PassiveSub->ClearAllPlayerData();
	}	
}

//ms: 연료 값을 가져오는 로직
float UAO_GameInstance::GetInitialFuel()
{
	// BP에서 지정한 에셋이 있다면 그 값을 반환
	if (FuelDataAsset)
	{
		return FuelDataAsset->InitialFuel;
	}
    
	// 에셋이 없으면 생성자에서 정한 기본값 반환
	return SharedTrainFuel;
}

float UAO_GameInstance::GetInitialFuelValue()
{
	// 1. 에디터에서 지정한 에셋(BP_FuelDataAsset)이 있는지 확인
	if (FuelDataAsset)
	{
		// 2. 에셋 내부의 InitialFuel 변수 값을 반환 (예: 80.0)
		return FuelDataAsset->InitialFuel;
	}
    
	// 3. 만약 에셋을 안 꽂았다면 방어용으로 기본값 반환
	return 40.0f;
}
// ms
