// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/PlayerState/AO_PlayerState.h"

#include "UI/Actor/AO_LobbyReadyBoardActor.h"
#include "Kismet/GameplayStatics.h"
#include "AO_Log.h"
#include "Game/GameMode/AO_GameMode_Stage.h"
#include "Net/UnrealNetwork.h"

AAO_PlayerState::AAO_PlayerState()
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	AO_LOG(LogJM, Log, TEXT("End"));
	bLobbyIsReady = false;
	LobbyJoinOrder = -1;
	bIsLobbyHost = false;

	CharacterCustomizingData.CharacterMeshType = ECharacterMesh::Elsa;

	CharacterCustomizingData.HairOptionData.ParameterName = TEXT("HairStyle");
	CharacterCustomizingData.HairOptionData.OptionName = TEXT("Hair01");

	CharacterCustomizingData.ClothOptionData.ParameterName = TEXT("ClothType");
	CharacterCustomizingData.ClothOptionData.OptionName = TEXT("Glacier");
}

void AAO_PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAO_PlayerState, bLobbyIsReady);
	DOREPLIFETIME(AAO_PlayerState, LobbyJoinOrder);
	DOREPLIFETIME(AAO_PlayerState, bIsLobbyHost);
	DOREPLIFETIME(AAO_PlayerState, bIsAlive);	// JM : 생존 여부 확인용
	DOREPLIFETIME(AAO_PlayerState, CharacterCustomizingData);
	DOREPLIFETIME(AAO_PlayerState, PersistentInventory); // ms : 인벤토리
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

void AAO_PlayerState::OnRep_IsAlive()
{
	if (!bIsAlive)
	{
		AO_LOG(LogJM, Log, TEXT("Updated bIsAlive true -> false"));
	}
	else
	{
		AO_LOG(LogJM, Log, TEXT("Update bIsAlive false -> true"));
	}
}

void AAO_PlayerState::SetIsAlive(bool bInIsAlive)
{
	if (HasAuthority())
	{
		if (bIsAlive != bInIsAlive)
		{
			bIsAlive = bInIsAlive;
			OnRep_IsAlive();
			
			if (UWorld* World = GetWorld())
			{
				if (AAO_GameMode_Stage* StageGM = World->GetAuthGameMode<AAO_GameMode_Stage>())
				{
					StageGM->NotifyPlayerAliveStateChanged(this);
				}
			}
		}
	}
}

void AAO_PlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	TObjectPtr<AAO_PlayerState> PS = Cast<AAO_PlayerState>(PlayerState);

	if (PS)
	{
		PS->CharacterCustomizingData = this->CharacterCustomizingData;
	}
	//ms : 다음스테이지에서 인벤토리 유지
	if (AAO_PlayerState* NewPS = Cast<AAO_PlayerState>(PlayerState))
	{
		NewPS->PersistentInventory = PersistentInventory;
	}
	//ms
}

void AAO_PlayerState::ServerRPC_SetCharacterCustomizingData_Implementation(const FCustomizingData& CustomizingData)
{
	CharacterCustomizingData = CustomizingData;
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

//ms: 인벤토리 유지
void AAO_PlayerState::SaveInventoryBeforeTravel(UAO_InventoryComponent* Inv)
{
	PersistentInventory = Inv->Slots;
}