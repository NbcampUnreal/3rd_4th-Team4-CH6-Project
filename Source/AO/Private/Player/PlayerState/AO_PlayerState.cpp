// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/PlayerState/AO_PlayerState.h"

#include "UI/Actor/AO_LobbyReadyBoardActor.h"
#include "Kismet/GameplayStatics.h"
#include "AO_Log.h"
#include "Net/UnrealNetwork.h"

AAO_PlayerState::AAO_PlayerState()
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	AO_LOG(LogJM, Log, TEXT("End"));
	bLobbyIsReady = false;
	LobbyJoinOrder = -1;
	bIsLobbyHost = false;
}

void AAO_PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAO_PlayerState, bLobbyIsReady);
	DOREPLIFETIME(AAO_PlayerState, LobbyJoinOrder);
	DOREPLIFETIME(AAO_PlayerState, bIsLobbyHost);
}

/* ==================== 로비 레디 상태 ==================== */

void AAO_PlayerState::SetLobbyReady(bool bNewReady)
{
	if(bLobbyIsReady == bNewReady)
	{
		return;
	}

	bLobbyIsReady = bNewReady;
}

bool AAO_PlayerState::IsLobbyReady() const
{
	return bLobbyIsReady;
}

void AAO_PlayerState::OnRep_LobbyIsReady()
{
	// 블루프린트/위젯 바인딩용 델리게이트
	OnLobbyReadyChanged.Broadcast(bLobbyIsReady);

	// 보드 재빌드
	RefreshLobbyReadyBoard();
}

/* ==================== 로비 입장 순서 / 호스트 ==================== */

void AAO_PlayerState::SetLobbyJoinOrder(int32 InOrder)
{
	if(LobbyJoinOrder == InOrder)
	{
		return;
	}

	LobbyJoinOrder = InOrder;
}

int32 AAO_PlayerState::GetLobbyJoinOrder() const
{
	return LobbyJoinOrder;
}

void AAO_PlayerState::SetIsLobbyHost(bool bNewIsHost)
{
	if(bIsLobbyHost == bNewIsHost)
	{
		return;
	}

	bIsLobbyHost = bNewIsHost;
}

bool AAO_PlayerState::IsLobbyHost() const
{
	return bIsLobbyHost;
}

void AAO_PlayerState::OnRep_LobbyJoinOrder()
{
	// 호스트 / 순서 변경 시에도 보드 갱신
	RefreshLobbyReadyBoard();
}

void AAO_PlayerState::OnRep_IsLobbyHost()
{
	RefreshLobbyReadyBoard();
}

/* ==================== 이름 복제 ==================== */

// 이름이 복제될 때 호출됨
void AAO_PlayerState::OnRep_PlayerName()
{
	Super::OnRep_PlayerName();

	AO_LOG(LogJSH, Log, TEXT("OnRep_PlayerName: %s"), *GetPlayerName());

	// 보드에 표시되는 이름 갱신
	RefreshLobbyReadyBoard();
}

void AAO_PlayerState::BeginPlay()
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	Super::BeginPlay();
	AO_LOG(LogJM, Log, TEXT("End"));
}

void AAO_PlayerState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	Super::EndPlay(EndPlayReason);
	AO_LOG(LogJM, Log, TEXT("End"));
}

/* ==================== 보드 재빌드 ==================== */

// 현재 월드에 있는 모든 ReadyBoardActor에게 Rebuild 요청
void AAO_PlayerState::RefreshLobbyReadyBoard()
{
	UWorld* World = GetWorld();
	if(World == nullptr)
	{
		return;
	}

	TArray<AActor*> Boards;
	UGameplayStatics::GetAllActorsOfClass(World, AAO_LobbyReadyBoardActor::StaticClass(), Boards);

	for(AActor* Actor : Boards)
	{
		AAO_LobbyReadyBoardActor* Board = Cast<AAO_LobbyReadyBoardActor>(Actor);
		if(Board == nullptr)
		{
			continue;
		}

		Board->RebuildBoard();
	}
}
