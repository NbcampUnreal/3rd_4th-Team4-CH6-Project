// AO_LobbyReadyBoardActor.cpp

#include "UI/Actor/AO_LobbyReadyBoardActor.h"

#include "Components/WidgetComponent.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Player/PlayerState/AO_PlayerState.h"
#include "AO/AO_Log.h"
#include "UI/Widget/AO_LobbyReadyBoardWidget.h"

AAO_LobbyReadyBoardActor::AAO_LobbyReadyBoardActor()
{
	PrimaryActorTick.bCanEverTick = false;

	BoardWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("BoardWidget"));
	SetRootComponent(BoardWidgetComponent);

	BoardWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	BoardWidgetComponent->SetDrawSize(FVector2D(800.0f, 400.0f));
	BoardWidgetComponent->SetTwoSided(true);

	bReplicates = true;

	AO_LOG(LogJSH, Log, TEXT("LobbyBoardActor::Constructor %s"), *GetName());
}

void AAO_LobbyReadyBoardActor::BeginPlay()
{
	Super::BeginPlay();

	AO_LOG(LogJSH, Log, TEXT("LobbyBoardActor::BeginPlay %s"), *GetName());

	RebuildBoard();
}

void AAO_LobbyReadyBoardActor::MulticastRebuildBoard_Implementation()
{
	AO_LOG(LogJSH, Log, TEXT("LobbyBoardActor::MulticastRebuildBoard on %s"), *GetName());

	RebuildBoard();
}

bool AAO_LobbyReadyBoardActor::IsEveryoneReadyExceptHost(const TArray<APlayerState*>& Players) const
{
	AO_LOG(LogJSH, Log, TEXT("LobbyBoardActor::IsEveryoneReadyExceptHost - Checking %d players"), Players.Num());

	if(Players.Num() <= 1)
	{
		AO_LOG(LogJSH, Log, TEXT(" → false: only %d players"), Players.Num());
		return false;
	}

	APlayerState* HostPS = nullptr;

	for(APlayerState* PS : Players)
	{
		if(AAO_PlayerState* AOPS = Cast<AAO_PlayerState>(PS))
		{
			if(AOPS->IsLobbyHost())
			{
				HostPS = PS;
				break;
			}
		}
	}

	AO_LOG(LogJSH, Log, TEXT(" → Host is %s"), *GetNameSafe(HostPS));

	if(HostPS == nullptr)
	{
		AO_LOG(LogJSH, Warning, TEXT(" → false: HostPS null"));
		return false;
	}

	for(APlayerState* PS : Players)
	{
		if(PS == nullptr)
		{
			AO_LOG(LogJSH, Warning, TEXT(" → skip null PS"));
			continue;
		}

		if(PS == HostPS)
		{
			AO_LOG(LogJSH, Log, TEXT(" → skip host (%s)"), *PS->GetName());
			continue;
		}

		AAO_PlayerState* AOPS = Cast<AAO_PlayerState>(PS);
		if(AOPS == nullptr)
		{
			AO_LOG(LogJSH, Warning, TEXT(" → false: AOPS null (%s)"), *PS->GetName());
			return false;
		}

		AO_LOG(LogJSH, Log, TEXT(" → Check Ready: %s = %d"), *PS->GetPlayerName(), AOPS->IsLobbyReady());

		if(!AOPS->IsLobbyReady())
		{
			AO_LOG(LogJSH, Log, TEXT(" → false: %s not ready"), *PS->GetPlayerName());
			return false;
		}
	}

	AO_LOG(LogJSH, Log, TEXT(" → true: all non-host players ready"));
	return true;
}

void AAO_LobbyReadyBoardActor::RebuildBoard()
{
	AO_LOG(LogJSH, Log, TEXT("LobbyBoardActor::RebuildBoard called on %s"), *GetName());

	UWorld* World = GetWorld();
	if(World == nullptr)
	{
		AO_LOG(LogJSH, Error, TEXT(" → Abort: World is NULL"));
		return;
	}

	AGameStateBase* GS = World->GetGameState();
	if(GS == nullptr)
	{
		AO_LOG(LogJSH, Error, TEXT(" → Abort: GameState is NULL"));
		return;
	}

	const TArray<APlayerState*>& Players = GS->PlayerArray;

	bool bAllReadyExceptHost = IsEveryoneReadyExceptHost(Players);

	TArray<FAOLobbyReadyBoardEntry> Entries;
	Entries.Reserve(Players.Num());

	for(APlayerState* PS : Players)
	{
		if(PS == nullptr)
		{
			continue;
		}

		AAO_PlayerState* AOPS = Cast<AAO_PlayerState>(PS);
		if(AOPS == nullptr)
		{
			continue;
		}

		const bool bIsHost = AOPS->IsLobbyHost();
		const bool bReady = AOPS->IsLobbyReady();
		const int32 JoinOrder = AOPS->GetLobbyJoinOrder();

		FAOLobbyReadyBoardEntry Row;
		Row.PlayerName    = PS->GetPlayerName();
		Row.bIsHost       = bIsHost;
		Row.StatusLabel   = bIsHost ? TEXT("게임시작") : TEXT("레디");
		Row.bStatusActive = bIsHost ? bAllReadyExceptHost : bReady;
		Row.JoinOrder     = JoinOrder;

		Entries.Add(Row);
	}

	// 항상 같은 순서로 보이게 정렬
	Entries.Sort(
		[](const FAOLobbyReadyBoardEntry& A, const FAOLobbyReadyBoardEntry& B)
		{
			// 1) 호스트는 항상 맨 위
			if(A.bIsHost != B.bIsHost)
			{
				return A.bIsHost; // true 가 앞으로
			}

			// 2) 그 다음은 입장 순서 (작을수록 먼저)
			return A.JoinOrder < B.JoinOrder;
		});

	ApplyToWidget(Entries);
}

void AAO_LobbyReadyBoardActor::ApplyToWidget(const TArray<FAOLobbyReadyBoardEntry>& Entries)
{
	AO_LOG(LogJSH, Log, TEXT("LobbyBoardActor::ApplyToWidget  Entries=%d"), Entries.Num());

	if(BoardWidgetComponent == nullptr)
	{
		AO_LOG(LogJSH, Error, TEXT(" → Abort: BoardWidgetComponent null"));
		return;
	}

	UUserWidget* UserWidget = BoardWidgetComponent->GetUserWidgetObject();
	if(UserWidget == nullptr)
	{
		AO_LOG(LogJSH, Warning, TEXT(" → Abort: UserWidget null"));
		return;
	}

	AO_LOG(LogJSH, Log, TEXT(" → UserWidget = %s"), *UserWidget->GetName());

	if(UAO_LobbyReadyBoardWidget* BoardWidget = Cast<UAO_LobbyReadyBoardWidget>(UserWidget))
	{
		AO_LOG(LogJSH, Log, TEXT(" → Calling SetEntries(%d)"), Entries.Num());
		BoardWidget->SetEntries(Entries);
	}
	else
	{
		AO_LOG(LogJSH, Error, TEXT(" → Error: UserWidget is NOT BoardWidget"));
	}
}
