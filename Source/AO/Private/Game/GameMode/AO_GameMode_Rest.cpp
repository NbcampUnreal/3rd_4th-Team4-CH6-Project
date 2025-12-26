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

void AAO_GameMode_Rest::HandleRestExitRequest(AController* Requester)
{
	if(!HasAuthority())
	{
		return;
	}

	UWorld* World = GetWorld();
	if(World == nullptr)
	{
		return;
	}

	UAO_GameInstance* AO_GI = World->GetGameInstance<UAO_GameInstance>();
	if(AO_GI == nullptr)
	{
		AO_LOG(LogJSH, Warning, TEXT("RestExit: GameInstance is not UAO_GameInstance"));
		return;
	}

	// 다음 스테이지로 인덱스 증가
	if(!AO_GI->TryAdvanceStageIndex())
	{
		// 더 이상 스테이지가 없으면 → 로비로 반환
		const FName LobbyMap = AO_GI->GetLobbyMap();
		if(!LobbyMap.IsNone())
		{
			const FString LobbyPath = LobbyMap.ToString() + TEXT("?listen");
			AO_GI->ResetRun();
			// World->ServerTravel(LobbyPath);
			// JM : voice crash 막기 위해 확인 후 레벨이동 시작
			RequestSynchronizedServerTravel(LobbyPath);
		}
		else
		{
			AO_LOG(LogJSH, Warning, TEXT("RestExit: LobbyMap is None"));
		}
		return;
	}

	const FName NextStageMap = AO_GI->GetCurrentStageMap();
	if(NextStageMap.IsNone())
	{
		AO_LOG(LogJSH, Warning, TEXT("RestExit: NextStageMap is None after TryAdvanceStageIndex"));
		return;
	}

	const FString Path = NextStageMap.ToString() + TEXT("?listen");
	AO_LOG(LogJSH, Log, TEXT("RestExit: Travel to %s"), *Path);
	// World->ServerTravel(Path);
	// JM : Voice crash 막기 위해 확인 후 레벨이동 시작
	RequestSynchronizedServerTravel(Path);
}
