// AO_GameMode_Stage.cpp (장주만)


#include "Game/GameMode/AO_GameMode_Stage.h"

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
