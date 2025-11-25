// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/GameMode/AO_GameMode_Lobby.h"
#include "Player/PlayerController/AO_PlayerController_Lobby.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "AO/AO_Log.h"
#include "Player/PlayerState/AO_PlayerState.h"
#include "UI/Actor/AO_LobbyReadyBoardActor.h"

AAO_GameMode_Lobby::AAO_GameMode_Lobby()
{
	PlayerControllerClass = AAO_PlayerController_Lobby::StaticClass();
	bUseSeamlessTravel = true;

	NextLobbyJoinOrder = 0;
}

void AAO_GameMode_Lobby::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// 로비에 이미 있는 플레이어들의 JoinOrder 를 보고 NextLobbyJoinOrder 재계산
	UpdateNextLobbyJoinOrderFromExistingPlayers();

	// 새로 들어온 플레이어가 아직 순서를 안 받았다면 부여
	if(NewPlayer != nullptr && NewPlayer->PlayerState != nullptr)
	{
		AssignJoinOrderIfNeeded(NewPlayer->PlayerState);
	}

	NotifyLobbyBoardChanged();
}

void AAO_GameMode_Lobby::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	// 나간다고 해서 기존 순서를 재배치하지는 않음 (공백만 생김)
	NotifyLobbyBoardChanged();
}

void AAO_GameMode_Lobby::HandleSeamlessTravelPlayer(AController*& C)
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	Super::HandleSeamlessTravelPlayer(C);

	// JM: 레벨 이동시 Voice Chat이 내부적으로 종료됨을 확인
	// 명시적으로 다시 StartVoiceChat을 호출하여 보이스 채팅 연결 활성화
	if (AAO_PlayerController_InGameBase* AO_PC_InGame = Cast<AAO_PlayerController_InGameBase>(C))
	{
		AO_PC_InGame->Client_StartVoiceChat();
	}
	else
	{
		AO_LOG(LogJM, Warning, TEXT("Can't cast to PC -> AAO InGame PC "));
	}
	AO_LOG(LogJM, Log, TEXT("End"));
}

/* ========== 로비 입장 순서 관리 ========== */

void AAO_GameMode_Lobby::UpdateNextLobbyJoinOrderFromExistingPlayers()
{
	if (GameState == nullptr)
	{
		return;
	}

	int32 MaxOrder = -1;

	for (APlayerState* PS : GameState->PlayerArray)
	{
		if (AAO_PlayerState* AOPS = Cast<AAO_PlayerState>(PS))
		{
			const int32 Order = AOPS->GetLobbyJoinOrder();
			if (Order >= 0 && Order > MaxOrder)
			{
				MaxOrder = Order;
			}
		}
	}

	NextLobbyJoinOrder = MaxOrder + 1;
}

void AAO_GameMode_Lobby::AssignJoinOrderIfNeeded(APlayerState* PlayerState)
{
	AAO_PlayerState* AOPS = Cast<AAO_PlayerState>(PlayerState);
	if (AOPS == nullptr)
	{
		return;
	}

	// 이미 순서를 가진 플레이어면(예: 스테이지 갔다가 로비로 돌아온 경우) 건너뜀
	if (AOPS->GetLobbyJoinOrder() >= 0)
	{
		return;
	}

	AOPS->SetLobbyJoinOrder(NextLobbyJoinOrder);
	++NextLobbyJoinOrder;

	AO_LOG(LogJSH, Log, TEXT("Lobby: Assign JoinOrder=%d to %s"),
		AOPS->GetLobbyJoinOrder(),
		*PlayerState->GetPlayerName());
}

/* ========== 레디 상태/호스트/시작 처리 ========== */

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
	if (!GameState)
	{
		return nullptr;
	}

	for (APlayerState* PS : GameState->PlayerArray)
	{
		if (AAO_PlayerState* AOPS = Cast<AAO_PlayerState>(PS))
		{
			if (AOPS->IsLobbyHost())
			{
				return PS->GetOwner<AController>();
			}
		}
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

	// JM : 레벨 이동 전에 보이스 채팅 비활성화 (Crash 발생 방지) - 이게 UX 안좋을것 같은데... 달리 방법이...
	// 근데 자꾸 크래쉬 발생해서 넣긴 했음...
	StopVoiceChatForAllClients();

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

void AAO_GameMode_Lobby::StopVoiceChatForAllClients()
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	if (UWorld* World = GetWorld())
	{
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			AAO_PlayerController_Lobby* AO_PC_Lobby = Cast<AAO_PlayerController_Lobby>(*It);
			if (AO_PC_Lobby)
			{
				AO_PC_Lobby->Client_StopVoiceChat();
			}
			else
			{
				AO_LOG(LogJM, Warning, TEXT("Can't Cast to PC_Lobby"));
			}
		}
	}
	else
	{
		AO_LOG(LogJM, Warning, TEXT("No World"));
	}
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_GameMode_Lobby::TravelToStage()
{
	if (UWorld* World = GetWorld())
	{
		FString Path = TEXT("/Game/AVaOut/Maps/LV_Meadow?listen");
		World->ServerTravel(Path);
	}
}
