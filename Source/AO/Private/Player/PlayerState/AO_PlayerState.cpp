// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/PlayerState/AO_PlayerState.h"

#include "UI/Actor/AO_LobbyReadyBoardActor.h"
#include "Kismet/GameplayStatics.h"
#include "AO_Log.h"
#include "Game/AO_MapRoutes.h"
#include "Game/GameInstance/AO_GameInstance.h"
#include "Game/GameMode/AO_GameMode_Stage.h"
#include "Game/GameState/AO_GameState.h"
#include "Player/PlayerController/AO_PlayerController_Stage.h"
#include "Net/UnrealNetwork.h"
#include "Online/AO_OnlineSessionSubsystem.h"
#include "Settings/AO_GameSettingsManager.h"
#include "Settings/AO_GameUserSettings.h"

AAO_PlayerState::AAO_PlayerState()
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	AO_LOG(LogJM, Log, TEXT("End"));
	bLobbyIsReady = false;
	LobbyJoinOrder = -1;
	bIsLobbyHost = false;
	DeathCount = 0;

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
	DOREPLIFETIME(AAO_PlayerState, DeathCount);
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
	
	APlayerController* OwnerPC = Cast<APlayerController>(GetOwner());
	if (OwnerPC != nullptr)
	{
		AAO_PlayerController_Stage* StagePC = Cast<AAO_PlayerController_Stage>(OwnerPC);
		if (StagePC != nullptr)
		{
			if (OwnerPC->IsLocalController())
			{
				UWorld* World = GetWorld();
				AAO_GameState* AO_GS = nullptr;
				if (World != nullptr)
				{
					AO_GS = World->GetGameState<AAO_GameState>();
				}

				if (!bIsAlive)
				{
					// 사망 시점에 공용 부활칩이 1개 이상 있으면 → 자동 부활 대기 상태로 간주
					if (AO_GS != nullptr && AO_GS->GetSharedReviveCount() > 0)
					{
						StagePC->bPendingAutoRespawn = true;
						StagePC->StartRespawnCountdown(5.0f);
					}
					else
					{
						// 부활칩이 없으면 → 자동 부활 없음, 카운트다운도 사용 안 함
						StagePC->bPendingAutoRespawn = false;
						StagePC->StopRespawnCountdown();
					}
				}
				else
				{
					// 다시 살아남 → 자동 부활 대기 해제 + 카운트다운 정리
					StagePC->bPendingAutoRespawn = false;
					StagePC->StopRespawnCountdown();
				}
			}
		}
	}
}

void AAO_PlayerState::OnRep_DeathCount()
{
	AO_LOG_ROLE(LogJM, Log, TEXT("Start"));

	if (UAO_DelegateManager* DM = GetGameInstance()->GetSubsystem<UAO_DelegateManager>())
	{
		DM->OnStatisticsUpdated.Broadcast();
	}
	
	AO_LOG_ROLE(LogJM, Log, TEXT("End"));
}

void AAO_PlayerState::AddDeathCount()
{
	AO_LOG_ROLE(LogJM, Log, TEXT("Start"));

	if (HasAuthority())
	{
		DeathCount++;
		OnRep_DeathCount();	// JM : 서버에서는 호출되지 않기에 명시적으로 실행
	}
	
	AO_LOG_ROLE(LogJM, Log, TEXT("End"));
}

void AAO_PlayerState::SetIsAlive(bool bInIsAlive)
{
	if (HasAuthority())
	{
		if (bIsAlive != bInIsAlive)
		{
			bIsAlive = bInIsAlive;
			OnRep_IsAlive();

			if (!bInIsAlive)	// 사망한 경우
			{
				AddDeathCount();
			}
			
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
		//ms : 다음스테이지에서 인벤토리 유지
		if (bInventoryShouldPersist)
		{
			PS->PersistentInventory = this->PersistentInventory;
		}
		else
		{
			PS->PersistentInventory.Empty();
		}
		//ms
		PS->DeathCount = this->DeathCount;	// JM : 해당 캐릭터 죽음 횟수 유지
	}
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

	BroadcastPlayerNameReady();
}

void AAO_PlayerState::BeginPlay()
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	Super::BeginPlay();

	if (!GetPlayerName().IsEmpty())
	{
		OnPlayerNameReady.Broadcast(FText::FromString(GetPlayerName()));
	}

	InitVoiceChat();	// JM : 레벨 이동시 보이스 채팅 초기화 (Unmute 해제)

	if (HasAuthority())
	{
		FString CurrentMapName = GetWorld()->GetMapName();
		FString LobbyName = FPackageName::GetShortName(AO_MapRoutes::LOBBY_MAP);
		if (CurrentMapName.Contains(LobbyName))
		{
			DeathCount = 0;
		}
	}
	
	
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

void AAO_PlayerState::InitVoiceChat()
{
	AO_LOG_ROLE(LogJM, Warning, TEXT("Start"));
	UAO_GameUserSettings* GameUserSettings = GetGameInstance()->GetSubsystem<UAO_GameSettingsManager>()->GetGameUserSettings();
	if (!AO_ENSURE(GameUserSettings, TEXT("Can't Get GameUserSettings")))
	{
		return;
	}

	UAO_OnlineSessionSubsystem* OSS = GetGameInstance()->GetSubsystem<UAO_OnlineSessionSubsystem>();
	if (!AO_ENSURE(OSS, TEXT("Can't Get OSS")))
	{
		return;
	}

	AO_LOG_ROLE(LogJM, Warning, TEXT("PS(%s) Voice Enabled (%d)"), *GetName(), GameUserSettings->bIsEnableVoiceChat);
	OSS->UnmuteAllRemoteTalker();	// JM : 보이스 활성화 안해도 Unmute는 해야 들리겠지?
	if (GameUserSettings->bIsEnableVoiceChat)
	{
		OSS->StartVoiceChat();
	}
	AO_LOG_ROLE(LogJM, Warning, TEXT("End"));
}

void AAO_PlayerState::BroadcastPlayerNameReady()
{
	const FString Name = GetPlayerName();
	if (!Name.IsEmpty())
	{
		OnPlayerNameReady.Broadcast(FText::FromString(Name));
	}
}

//ms: 인벤토리 유지
void AAO_PlayerState::SaveInventoryBeforeTravel(UAO_InventoryComponent* Inv)
{
	PersistentInventory = Inv->Slots;
}

void AAO_PlayerState::ResetStateInventory()
{
	PersistentInventory.Empty();
}

void AAO_PlayerState::SetSafeZoneState(bool bInZone)
{
	bInsideTravelSafeZone = bInZone;
}