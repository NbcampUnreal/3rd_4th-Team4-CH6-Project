// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/GameMode/AO_GameMode_Lobby.h"
#include "Player/PlayerController/AO_PlayerController_Lobby.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "AO/AO_Log.h"
#include "Game/GameInstance/AO_GameInstance.h"
#include "Player/PlayerState/AO_PlayerState.h"
#include "UI/Actor/AO_LobbyReadyBoardActor.h"

AAO_GameMode_Lobby::AAO_GameMode_Lobby()
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	PlayerControllerClass = AAO_PlayerController_Lobby::StaticClass();
	bUseSeamlessTravel = true;

	NextLobbyJoinOrder = 0;
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_GameMode_Lobby::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// 로비에 이미 있는 플레이어들의 JoinOrder 를 보고 NextLobbyJoinOrder 재계산
	UpdateNextLobbyJoinOrderFromExistingPlayers();

	// 새로 들어온 플레이어가 아직 순서를 안 받았다면 부여 + 호스트 여부 결정
	if(NewPlayer != nullptr && NewPlayer->PlayerState != nullptr)
	{
		AssignJoinOrderIfNeeded(NewPlayer->PlayerState);

		UWorld* World = GetWorld();
		if(World != nullptr)
		{
			UAO_GameInstance* AO_GI = World->GetGameInstance<UAO_GameInstance>();
			AAO_PlayerState* AOPS = Cast<AAO_PlayerState>(NewPlayer->PlayerState);

			if(AO_GI != nullptr && AOPS != nullptr)
			{
				// 1) 아직 호스트가 정해져 있지 않으면 → 이 사람을 호스트로 기록
				if(AO_GI->HasLobbyHost() == false)
				{
					AO_GI->SetLobbyHostFromPlayerState(AOPS);
					AOPS->SetIsLobbyHost(true);
				}
				else
				{
					// 2) 이미 GI에 호스트가 있으면 → UniqueNetId 비교로 호스트 여부 설정
					const bool bIsHost = AO_GI->IsLobbyHostPlayerState(AOPS);
					AOPS->SetIsLobbyHost(bIsHost);
				}
			}
		}
	}

	NotifyLobbyBoardChanged();
}

void AAO_GameMode_Lobby::HandleSeamlessTravelPlayer(AController*& C)
{
	Super::HandleSeamlessTravelPlayer(C);

	if(C == nullptr)
	{
		return;
	}

	AAO_PlayerState* AOPS = C->GetPlayerState<AAO_PlayerState>();
	if(AOPS == nullptr)
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
		return;
	}

	// 필요하다면 로비 복귀 시에도 JoinOrder 재부여
	AssignJoinOrderIfNeeded(AOPS);

	const bool bIsHost = AO_GI->IsLobbyHostPlayerState(AOPS);
	AOPS->SetIsLobbyHost(bIsHost);

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
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_GameMode_Lobby::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	Super::EndPlay(EndPlayReason);
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

void AAO_GameMode_Lobby::TravelToStage()
{
	UWorld* World = GetWorld();
	if(World == nullptr)
	{
		return;
	}

	UAO_GameInstance* AO_GI = World->GetGameInstance<UAO_GameInstance>();
	if(AO_GI == nullptr)
	{
		AO_LOG(LogJSH, Warning, TEXT("Lobby: TravelToStage: GameInstance is not UAO_GameInstance"));
		return;
	}

	// 새 판 시작 : 스테이지 인덱스 / 연료 초기화
	AO_GI->ResetRun();

	const FName StageMapName = AO_GI->GetCurrentStageMap();
	if(StageMapName.IsNone())
	{
		AO_LOG(LogJSH, Warning, TEXT("Lobby: TravelToStage: StageMapName is None"));
		return;
	}

	const FString Path = StageMapName.ToString() + TEXT("?listen");
	AO_LOG(LogJSH, Log, TEXT("Lobby: TravelToStage → %s"), *Path);

	World->ServerTravel(Path);
}