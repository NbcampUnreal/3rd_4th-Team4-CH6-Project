// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/GameMode/AO_GameMode_Lobby.h"
#include "Player/PlayerController/AO_PlayerController_Lobby.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "AO/AO_Log.h"
#include "UI/Actor/AO_LobbyReadyBoardActor.h"

AAO_GameMode_Lobby::AAO_GameMode_Lobby()
{
	PlayerControllerClass = AAO_PlayerController_Lobby::StaticClass();
	bUseSeamlessTravel = true;
}

void AAO_GameMode_Lobby::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	NotifyLobbyBoardChanged();
}

void AAO_GameMode_Lobby::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	
	NotifyLobbyBoardChanged();
}

void AAO_GameMode_Lobby::SetPlayerReady(AController* Controller, bool bReady)
{
	if (!Controller)
	{
		return;
	}

	if (bReady)
	{
		ReadyPlayers.Add(Controller);
	}
	else
	{
		ReadyPlayers.Remove(Controller);
	}

	AO_LOG(LogJSH, Log, TEXT("Lobby: %s Ready=%d, ReadyCount=%d"),
		*Controller->GetName(),
		static_cast<int32>(bReady),
		ReadyPlayers.Num());

	NotifyLobbyBoardChanged();
}

AController* AAO_GameMode_Lobby::GetHostController() const
{
	// PlayerArray[0] 를 호스트로 간주
	if (GameState && GameState->PlayerArray.Num() > 0 && GameState->PlayerArray[0])
	{
		return GameState->PlayerArray[0]->GetOwner<AController>();
	}

	return nullptr;
}

bool AAO_GameMode_Lobby::IsEveryoneReadyExceptHost() const
{
	if (!GameState)
	{
		return false;
	}

	const int32 TotalPlayers = GameState->PlayerArray.Num();

	// 최소 2명 이상 있어야 시작 가능
	if (TotalPlayers <= 1)
	{
		return false;
	}

	AController* Host = GetHostController();
	if (!Host)
	{
		return false;
	}

	for (APlayerState* PS : GameState->PlayerArray)
	{
		if (!PS)
		{
			continue;
		}

		AController* Ctrl = PS->GetOwner<AController>();
		if (!Ctrl || Ctrl == Host)
		{
			continue;
		}

		if (!ReadyPlayers.Contains(Ctrl))
		{
			return false;
		}
	}

	return true;
}

void AAO_GameMode_Lobby::RequestStartFrom(AController* Controller)
{
	if (!Controller)
	{
		return;
	}

	AController* Host = GetHostController();

	// 호스트만 시작 가능
	if (Controller != Host)
	{
		AO_LOG(LogJSH, Warning, TEXT("Lobby: Only host can start (Caller=%s, Host=%s)"),
			*Controller->GetName(),
			Host ? *Host->GetName() : TEXT("None"));
		return;
	}

	// 호스트 제외 모두 레디 확인
	if (!IsEveryoneReadyExceptHost())
	{
		AO_LOG(LogJSH, Warning, TEXT("Lobby: Not everyone is ready, cannot start"));
		return;
	}

	AO_LOG(LogJSH, Log, TEXT("Lobby: All players ready, starting stage"));
	TravelToStage();
}

void AAO_GameMode_Lobby::NotifyLobbyBoardChanged()
{
	UWorld* World = GetWorld();
	if(!World)
	{
		return;
	}

	TArray<AActor*> FoundBoards;
	UGameplayStatics::GetAllActorsOfClass(World, AAO_LobbyReadyBoardActor::StaticClass(), FoundBoards);

	for(AActor* Actor : FoundBoards)
	{
		if(AAO_LobbyReadyBoardActor* Board = Cast<AAO_LobbyReadyBoardActor>(Actor))
		{
			Board->MulticastRebuildBoard();
		}
	}
}

void AAO_GameMode_Lobby::TravelToStage()
{
	if (UWorld* World = GetWorld())
	{
		FString Path = "/Game/AVaOut/Maps/LV_Meadow?listen";
		World->ServerTravel(Path);
	}
}
