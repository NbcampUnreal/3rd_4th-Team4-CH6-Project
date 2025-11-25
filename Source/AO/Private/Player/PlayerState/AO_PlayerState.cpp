// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/PlayerState/AO_PlayerState.h"
#include "UI/Actor/AO_LobbyReadyBoardActor.h"
#include "Kismet/GameplayStatics.h"
#include "AO_Log.h"
#include "Net/UnrealNetwork.h"

AAO_PlayerState::AAO_PlayerState()
{
	bLobbyIsReady = false;
}

void AAO_PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAO_PlayerState, bLobbyIsReady);
}

/* ==================== 로비 레디 상태 ==================== */

void AAO_PlayerState::SetLobbyReady(bool bNewReady)
{
	if(bLobbyIsReady == bNewReady)
	{
		return;
	}
	bLobbyIsReady = bNewReady;

	OnRep_LobbyIsReady();
}

bool AAO_PlayerState::IsLobbyReady() const
{
	return bLobbyIsReady;
}

void AAO_PlayerState::OnRep_LobbyIsReady()
{
	OnLobbyReadyChanged.Broadcast(bLobbyIsReady);
	
	RefreshLobbyReadyBoard();
}

// 이름이 복제될 때 호출됨
void AAO_PlayerState::OnRep_PlayerName()
{
	Super::OnRep_PlayerName();

	AO_LOG(LogJSH, Log, TEXT("OnRep_PlayerName: %s"), *GetPlayerName());

	RefreshLobbyReadyBoard();
}

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