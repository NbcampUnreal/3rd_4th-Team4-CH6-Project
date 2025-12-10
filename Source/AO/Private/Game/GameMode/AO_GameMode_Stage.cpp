// AO_GameMode_Stage.cpp (장주만)


#include "Game/GameMode/AO_GameMode_Stage.h"
#include "Game/GameInstance/AO_GameInstance.h"
#include "AO_Log.h"

AAO_GameMode_Stage::AAO_GameMode_Stage()
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_GameMode_Stage::PostLogin(APlayerController* NewPlayer)
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	Super::PostLogin(NewPlayer);
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_GameMode_Stage::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId,
	FString& ErrorMessage)
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_GameMode_Stage::HandleSeamlessTravelPlayer(AController*& C)
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	Super::HandleSeamlessTravelPlayer(C);
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_GameMode_Stage::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	Super::EndPlay(EndPlayReason);
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_GameMode_Stage::HandleStageExitRequest(AController* Requester)
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
		AO_LOG(LogJSH, Warning, TEXT("StageExit: GameInstance is not UAO_GameInstance"));
		return;
	}

	const float Fuel = AO_GI->SharedTrainFuel;
	constexpr float RequiredFuel = 20.0f;

	if(Fuel < RequiredFuel)
	{
		AO_LOG(LogJSH, Log, TEXT("StageExit: Not enough fuel. Fuel=%.1f, Required=%.1f"), Fuel, RequiredFuel);
		return;
	}

	AO_LOG(LogJSH, Log, TEXT("StageExit: OK, Fuel=%.1f → Travel to next map"), Fuel);

	FName TargetMapName;

	// 마지막 스테이지면 → 로비로 귀환 (엔딩 분기는 로비에서 처리)
	if(AO_GI->IsLastStage())
	{
		TargetMapName = AO_GI->GetLobbyMap();

		// 다음 판은 다시 처음부터
		AO_GI->ResetRun();
	}
	else
	{
		// 아니면 휴게공간으로 이동
		TargetMapName = AO_GI->GetRestMap();
	}

	if(TargetMapName.IsNone())
	{
		AO_LOG(LogJSH, Warning, TEXT("StageExit: TargetMapName is None"));
		return;
	}

	const FString Path = TargetMapName.ToString() + TEXT("?listen");
	World->ServerTravel(Path);
}

void AAO_GameMode_Stage::HandleStageFail(AController* Requester)
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
		AO_LOG(LogJSH, Warning, TEXT("StageFail: GameInstance is not UAO_GameInstance"));
		return;
	}

	AO_LOG(LogJSH, Log, TEXT("StageFail: Requested by %s, Fuel=%.1f, StageIndex=%d"),
		Requester ? *Requester->GetName() : TEXT("None"),
		AO_GI->SharedTrainFuel,
		AO_GI->CurrentStageIndex);

	// 다음 판은 항상 처음부터
	AO_GI->ResetRun();

	const FName LobbyMapName = AO_GI->GetLobbyMap();
	if(LobbyMapName.IsNone())
	{
		AO_LOG(LogJSH, Warning, TEXT("StageFail: LobbyMapName is None"));
		return;
	}

	const FString Path = LobbyMapName.ToString() + TEXT("?listen");
	AO_LOG(LogJSH, Log, TEXT("StageFail: Travel to %s"), *Path);

	World->ServerTravel(Path);
}