// Fill out your copyright notice in the Description page of Project Settings.

#include "Online/AO_OnlineSessionSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Online/OnlineSessionNames.h"
#include "Misc/SecureHash.h"
#include "Engine/Engine.h"
#include "Engine/EngineBaseTypes.h"
#include "AO/AO_Log.h"
#include "Interfaces/VoiceInterface.h"	// JM : VoiceInterface
#include "Player/PlayerState/AO_PlayerState.h"

namespace
{
	static bool IsNullOSS()
	{
		if (const IOnlineSubsystem* OSS = IOnlineSubsystem::Get())
		{
			return OSS->GetSubsystemName() == FName(TEXT("NULL"));
		}
		return false;
	}
}

using namespace AO_SessionKeys;

UAO_OnlineSessionSubsystem::UAO_OnlineSessionSubsystem()
{
}

void UAO_OnlineSessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (IOnlineSessionPtr Session = GetSessionInterface(); Session.IsValid())
	{
		InviteAcceptedHandle = Session->AddOnSessionUserInviteAcceptedDelegate_Handle(
			FOnSessionUserInviteAcceptedDelegate::CreateUObject(
				this, &ThisClass::OnSessionUserInviteAccepted));
	}
	else
	{
		AO_LOG(LogJSH, Warning, TEXT("Initialize: Session interface invalid, invite delegate not bound"));
	}

	if (GEngine && !NetFailHandle.IsValid())
	{
		NetFailHandle = GEngine->OnNetworkFailure().AddUObject(this, &ThisClass::HandleNetworkFailure);
	}
	else if (!GEngine)
	{
		AO_LOG(LogJSH, Warning, TEXT("Initialize: GEngine is null, network failure delegate not bound"));
	}
}

void UAO_OnlineSessionSubsystem::Deinitialize()
{
	if (IOnlineSessionPtr Session = GetSessionInterface(); Session.IsValid())
	{
		if (InviteAcceptedHandle.IsValid())
		{
			Session->ClearOnSessionUserInviteAcceptedDelegate_Handle(InviteAcceptedHandle);
			InviteAcceptedHandle.Reset();
		}
	}
	else
	{
		AO_LOG(LogJSH, Warning, TEXT("Deinitialize: Session interface invalid, invite delegate not cleared"));
	}

	if (GEngine && NetFailHandle.IsValid())
	{
		GEngine->OnNetworkFailure().Remove(NetFailHandle);
		NetFailHandle.Reset();
	}

	// JM : 종료시 보이스 세션 정리 로직
	if (IOnlineVoicePtr VoiceInterface = GetOnlineVoiceInterface())
	{
		VoiceInterface->Shutdown();
	}
	else
	{
		AO_LOG(LogJM, Warning, TEXT("Cant Get Voice Interface"));
	}
	
	
	Super::Deinitialize();
}

/* ==================== 유틸 ==================== */
FString UAO_OnlineSessionSubsystem::ToMD5(const FString& In)
{
	return FMD5::HashAnsiString(*In);
}

IOnlineSessionPtr UAO_OnlineSessionSubsystem::GetSessionInterface() const
{
	if (const IOnlineSubsystem* OSS = IOnlineSubsystem::Get())
	{
		return OSS->GetSessionInterface();
	}

	AO_LOG(LogJSH, Warning, TEXT("GetSessionInterface: OnlineSubsystem is null"));
	return nullptr;
}

IOnlineVoicePtr UAO_OnlineSessionSubsystem::GetOnlineVoiceInterface() const
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	if (const IOnlineSubsystem* OSS = IOnlineSubsystem::Get())		// JM : raw pointer 타입으로 반환됨
	{
		AO_LOG(LogJM, Log, TEXT("return OSS::Voice Interface"));
		return OSS->GetVoiceInterface();
	}
	AO_LOG(LogJM, Warning, TEXT("OSS is Null: return nullptr"));
	return nullptr;
}

// JM NOTE : 이렇게 Depth 가 깊어지는 경우 Early Return 방식을 쓰면 코드가 조금 더 깔끔해집니다
bool UAO_OnlineSessionSubsystem::IsLocalHost() const
{
	if (IOnlineSessionPtr S = GetSessionInterface(); S.IsValid())
	{
		if (const FNamedOnlineSession* NS = S->GetNamedSession(NAME_GameSession))
		{
			if (const IOnlineSubsystem* OSS = IOnlineSubsystem::Get())
			{
				if (IOnlineIdentityPtr Identity = OSS->GetIdentityInterface(); Identity.IsValid())
				{
					TSharedPtr<const FUniqueNetId> LocalId = Identity->GetUniquePlayerId(0);
					if (LocalId.IsValid() && NS->OwningUserId.IsValid())
					{
						return *NS->OwningUserId == *LocalId;
					}

					if (!LocalId.IsValid())
					{
						AO_LOG(LogJSH, Warning, TEXT("IsLocalHost: LocalId invalid"));
					}
					if (!NS->OwningUserId.IsValid())
					{
						AO_LOG(LogJSH, Warning, TEXT("IsLocalHost: OwningUserId invalid"));
					}
				}
				else
				{
					AO_LOG(LogJSH, Warning, TEXT("IsLocalHost: Identity interface invalid"));
				}
			}
			else
			{
				AO_LOG(LogJSH, Warning, TEXT("IsLocalHost: OnlineSubsystem is null"));
			}
		}
		else
		{
			AO_LOG(LogJSH, Warning, TEXT("IsLocalHost: NamedSession not found"));
		}
	}
	else
	{
		AO_LOG(LogJSH, Warning, TEXT("IsLocalHost: Session interface invalid"));
	}
	return false;
}

void UAO_OnlineSessionSubsystem::HandleNetworkFailure(
	UWorld* World, UNetDriver* NetDriver,
	ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	AO_LOG(LogJSH, Warning, TEXT("[NetworkFailure] Code=%d, Msg=%s"),
		static_cast<int32>(FailureType), *ErrorString);

	// 연결 끊어지면 보이스 채팅 중지
	StopVoiceChat();

	// 세션 정리: 이후 조인/호스트 재시도 꼬임 방지
	if (IOnlineSessionPtr Session = GetSessionInterface(); Session.IsValid())
	{
		if (Session->GetNamedSession(NAME_GameSession) != nullptr)
		{
			Session->DestroySession(NAME_GameSession); // 콜백 대기 불필요
		}
	}
	else
	{
		AO_LOG(LogJSH, Warning, TEXT("HandleNetworkFailure: Session interface invalid, DestroySession skipped"));
	}

	// 내부 상태 리셋
	bFinding = false;
	bOpInProgress = false;
	bPendingRehost = false;
	bPendingInviteJoin = false;

	// 델리게이트 핸들 해제
	if (IOnlineSessionPtr S = GetSessionInterface(); S.IsValid())
	{
		if (CreateHandle.IsValid())
		{
			S->ClearOnCreateSessionCompleteDelegate_Handle(CreateHandle);
			CreateHandle.Reset();
		}
		if (FindHandle.IsValid())
		{
			S->ClearOnFindSessionsCompleteDelegate_Handle(FindHandle);
			FindHandle.Reset();
		}
		if (JoinHandle.IsValid())
		{
			S->ClearOnJoinSessionCompleteDelegate_Handle(JoinHandle);
			JoinHandle.Reset();
		}
		if (DestroyHandle.IsValid())
		{
			S->ClearOnDestroySessionCompleteDelegate_Handle(DestroyHandle);
			DestroyHandle.Reset();
		}
	}
	else
	{
		AO_LOG(LogJSH, Warning, TEXT("HandleNetworkFailure: Session interface invalid, delegate handles not cleared"));
	}
}

/* ==================== Host ==================== */
void UAO_OnlineSessionSubsystem::HostSession(int32 NumPublicConnections, bool bIsLAN)
{
	HostSessionEx(NumPublicConnections, bIsLAN, TEXT("New Room"), TEXT(""));
}

void UAO_OnlineSessionSubsystem::HostSessionEx(int32 NumPublicConnections, bool bIsLAN, const FString& RoomName, const FString& Password)
{
	if (bOpInProgress)
	{
		AO_LOG(LogJSH, Warning, TEXT("HostSessionEx: Operation in progress, request ignored (Room=%s)"), *RoomName);
		return;
	}
	bOpInProgress = true;

	IOnlineSessionPtr Session = GetSessionInterface();
	if (!Session.IsValid())
	{
		AO_LOG(LogJSH, Warning, TEXT("HostSessionEx: Session interface invalid, hosting aborted"));
		bOpInProgress = false;
		return;
	}

	/* 기존 세션이면 제거 후 재호스트 */
	if (Session->GetNamedSession(NAME_GameSession) != nullptr)
	{
		AO_LOG(LogJSH, Warning, TEXT("HostSessionEx: Existing session found, will destroy then rehost (Room=%s)"), *RoomName);

		CachedNumPublicConnections = NumPublicConnections;
		bCachedIsLAN = bIsLAN;
		CachedRoomName = RoomName;
		CachedPassword = Password;
		bPendingRehost = true;

		if (DestroyHandle.IsValid())
		{
			Session->ClearOnDestroySessionCompleteDelegate_Handle(DestroyHandle);
			DestroyHandle.Reset();
		}
		DestroyHandle = Session->AddOnDestroySessionCompleteDelegate_Handle(
			FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroyThenRecreateSession));

		if (!Session->DestroySession(NAME_GameSession))
		{
			AO_LOG(LogJSH, Warning, TEXT("HostSessionEx: DestroySession failed, rehost canceled"));
			Session->ClearOnDestroySessionCompleteDelegate_Handle(DestroyHandle);
			DestroyHandle.Reset();
			bOpInProgress = false;
		}
		return;
	}

	FOnlineSessionSettings Settings;
	Settings.bIsLANMatch = bIsLAN;
	Settings.NumPublicConnections = NumPublicConnections;
	Settings.bAllowJoinInProgress = true;
	Settings.bShouldAdvertise = true;
	Settings.bUsesPresence = true;
	Settings.bAllowJoinViaPresence = true;
	Settings.bUseLobbiesIfAvailable = true;

	const bool bHasPassword = !Password.IsEmpty();
	Settings.Set(KEY_SERVER_NAME, RoomName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	Settings.Set(KEY_HAS_PASSWORD, bHasPassword, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	Settings.Set(KEY_PASSWORD_MD5, ToMD5(Password), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	Settings.Set(SEARCH_LOBBIES, true, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	if (CreateHandle.IsValid())
	{
		Session->ClearOnCreateSessionCompleteDelegate_Handle(CreateHandle);
		CreateHandle.Reset();
	}
	CreateHandle = Session->AddOnCreateSessionCompleteDelegate_Handle(
		FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete));

	if (!Session->CreateSession(0, NAME_GameSession, Settings))
	{
		AO_LOG(LogJSH, Warning, TEXT("HostSessionEx: CreateSession returned false (Room=%s)"), *RoomName);
		Session->ClearOnCreateSessionCompleteDelegate_Handle(CreateHandle);
		CreateHandle.Reset();
		bOpInProgress = false;
	}
	else
	{
		AO_LOG(LogJSH, Log, TEXT("HostSessionEx: CreateSession requested (LAN=%d, NumPublic=%d, Room=%s, HasPw=%d)"),
			static_cast<int32>(bIsLAN),
			NumPublicConnections,
			*RoomName,
			static_cast<int32>(bHasPassword));
	}
}

void UAO_OnlineSessionSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (IOnlineSessionPtr Session = GetSessionInterface(); Session.IsValid())
	{
		if (CreateHandle.IsValid())
		{
			Session->ClearOnCreateSessionCompleteDelegate_Handle(CreateHandle);
			CreateHandle.Reset();
		}
	}
	else
	{
		AO_LOG(LogJSH, Warning, TEXT("OnCreateSessionComplete: Session interface invalid when clearing delegate"));
	}

	AO_LOG(LogJSH, Log, TEXT("OnCreateSessionComplete: SessionName=%s, Success=%d"),
		*SessionName.ToString(),
		static_cast<int32>(bWasSuccessful));

	if (!bWasSuccessful)
	{
		bOpInProgress = false;
		return;
	}

	if (UWorld* World = GetWorld())
	{
		UGameplayStatics::OpenLevel(World, GetLobbyMapName(), true, TEXT("?listen"));
	}
	else
	{
		AO_LOG(LogJSH, Warning, TEXT("OnCreateSessionComplete: World is null, cannot travel to lobby"));
	}

	bOpInProgress = false;
}

/* ==================== Find ==================== */
void UAO_OnlineSessionSubsystem::FindSessions(int32 MaxResults, bool bIsLAN)
{
	/* 진행 중이면 먼저 취소 */
	if (bFinding)
	{
		AO_LOG(LogJSH, Warning, TEXT("FindSessions: Already finding, CancelFind first"));
		CancelFind();
	}

	IOnlineSessionPtr Session = GetSessionInterface();
	if (!Session.IsValid())
	{
		AO_LOG(LogJSH, Warning, TEXT("FindSessions: Session interface invalid, search aborted"));
		OnFindSessionsCompleteEvent.Broadcast(false);
		return;
	}

	LastSearch = MakeShared<FOnlineSessionSearch>();
	LastSearch->MaxSearchResults = MaxResults;
	LastSearch->bIsLanQuery = bIsLAN;

	LastSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);

	if (FindHandle.IsValid())
	{
		Session->ClearOnFindSessionsCompleteDelegate_Handle(FindHandle);
		FindHandle.Reset();
	}
	FindHandle = Session->AddOnFindSessionsCompleteDelegate_Handle(
		FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete));

	bFinding = true;

	if (!Session->FindSessions(0, LastSearch.ToSharedRef()))
	{
		AO_LOG(LogJSH, Warning, TEXT("FindSessions: FindSessions returned false"));
		Session->ClearOnFindSessionsCompleteDelegate_Handle(FindHandle);
		FindHandle.Reset();
		bFinding = false;
		OnFindSessionsCompleteEvent.Broadcast(false);
	}
	else
	{
		AO_LOG(LogJSH, Log, TEXT("FindSessions: Requested (MaxResults=%d, LAN=%d)"),
			MaxResults,
			static_cast<int32>(bIsLAN));
	}
}

void UAO_OnlineSessionSubsystem::CancelFind()
{
	IOnlineSessionPtr Session = GetSessionInterface();
	if (Session.IsValid())
	{
		AO_LOG(LogJSH, Log, TEXT("CancelFind: CancelFindSessions requested"));
		Session->CancelFindSessions();
		if (FindHandle.IsValid())
		{
			Session->ClearOnFindSessionsCompleteDelegate_Handle(FindHandle);
			FindHandle.Reset();
		}
	}
	else
	{
		AO_LOG(LogJSH, Warning, TEXT("CancelFind: Session interface invalid, cancel skipped"));
	}
	LastSearch.Reset();
	bFinding = false;

	OnFindSessionsCompleteEvent.Broadcast(false);
}

void UAO_OnlineSessionSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (IOnlineSessionPtr Session = GetSessionInterface(); Session.IsValid())
	{
		if (FindHandle.IsValid())
		{
			Session->ClearOnFindSessionsCompleteDelegate_Handle(FindHandle);
			FindHandle.Reset();
		}
	}
	else
	{
		AO_LOG(LogJSH, Warning, TEXT("OnFindSessionsComplete: Session interface invalid when clearing delegate"));
	}

	bFinding = false;
	LastSearchResults.Reset();

	AO_LOG(LogJSH, Log, TEXT("OnFindSessionsComplete: Success=%d"), static_cast<int32>(bWasSuccessful));

	if (!bWasSuccessful || !LastSearch.IsValid())
	{
		if (!LastSearch.IsValid())
		{
			AO_LOG(LogJSH, Warning, TEXT("OnFindSessionsComplete: LastSearch invalid"));
		}
		OnFindSessionsCompleteEvent.Broadcast(false);
		return;
	}
	
	LastSearchResults = LastSearch->SearchResults;
	AO_LOG(LogJSH, Log, TEXT("OnFindSessionsComplete: RawResults=%d"), LastSearchResults.Num());
	
	{
		TSet<FString> SeenIds;
		LastSearchResults.RemoveAll(
			[&SeenIds](const FOnlineSessionSearchResult& R)
			{
				const FString Id = R.GetSessionIdStr();
				if (SeenIds.Contains(Id))
				{
					return true;
				}
				SeenIds.Add(Id);

				const auto& S = R.Session;

				bool bLobbyTag = false;
				S.SessionSettings.Get(SEARCH_LOBBIES, bLobbyTag);

				FString RoomName;
				const bool bHasName = S.SessionSettings.Get(KEY_SERVER_NAME, RoomName);

				const bool bBad =
					!bLobbyTag ||
					!bHasName || RoomName.IsEmpty() ||
					S.OwningUserName.IsEmpty();

				return bBad;
			});
	}

	AO_LOG(LogJSH, Log, TEXT("OnFindSessionsComplete: ValidResults=%d"), LastSearchResults.Num());

	OnFindSessionsCompleteEvent.Broadcast(true);
}

/* ==================== Join ==================== */
void UAO_OnlineSessionSubsystem::JoinSessionByIndex(int32 Index)
{
	if (bOpInProgress)
	{
		AO_LOG(LogJSH, Warning, TEXT("JoinSessionByIndex: Operation in progress, request ignored (Index=%d)"), Index);
		return;
	}
	if (!LastSearch.IsValid() || Index < 0 || Index >= LastSearchResults.Num())
	{
		AO_LOG(LogJSH, Warning, TEXT("JoinSessionByIndex: Invalid index or LastSearch (Index=%d, ResultCount=%d)"),
			Index, LastSearchResults.Num());
		return;
	}
	bOpInProgress = true;

	IOnlineSessionPtr Session = GetSessionInterface();
	if (!Session.IsValid())
	{
		AO_LOG(LogJSH, Warning, TEXT("JoinSessionByIndex: Session interface invalid, join aborted"));
		bOpInProgress = false;
		return;
	}

	if (JoinHandle.IsValid())
	{
		Session->ClearOnJoinSessionCompleteDelegate_Handle(JoinHandle);
		JoinHandle.Reset();
	}
	JoinHandle = Session->AddOnJoinSessionCompleteDelegate_Handle(
		FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete));

	if (!Session->JoinSession(0, NAME_GameSession, LastSearchResults[Index]))
	{
		AO_LOG(LogJSH, Warning, TEXT("JoinSessionByIndex: JoinSession returned false (Index=%d)"), Index);
		Session->ClearOnJoinSessionCompleteDelegate_Handle(JoinHandle);
		JoinHandle.Reset();
		bOpInProgress = false;
	}
	else
	{
		AO_LOG(LogJSH, Log, TEXT("JoinSessionByIndex: JoinSession requested (Index=%d)"), Index);
	}
}

void UAO_OnlineSessionSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSessionPtr Session = GetSessionInterface();
	if (!Session.IsValid())
	{
		AO_LOG(LogJSH, Warning, TEXT("OnJoinSessionComplete: Session interface invalid"));
		bOpInProgress = false;
		return;
	}

	if (JoinHandle.IsValid())
	{
		Session->ClearOnJoinSessionCompleteDelegate_Handle(JoinHandle);
		JoinHandle.Reset();
	}

	AO_LOG(LogJSH, Log, TEXT("OnJoinSessionComplete: SessionName=%s, Result=%d"),
		*SessionName.ToString(),
		static_cast<int32>(Result));

	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		bOpInProgress = false;
		return;
	}

	FString ConnectString;
	if (!Session->GetResolvedConnectString(SessionName, ConnectString))
	{
		AO_LOG(LogJSH, Warning, TEXT("OnJoinSessionComplete: GetResolvedConnectString failed"));
		bOpInProgress = false;
		return;
	}

	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		AO_LOG(LogJSH, Log, TEXT("OnJoinSessionComplete: ClientTravel to %s"), *ConnectString);
		PC->ClientTravel(ConnectString, TRAVEL_Absolute);
	}
	else
	{
		AO_LOG(LogJSH, Warning, TEXT("OnJoinSessionComplete: PlayerController is null, cannot ClientTravel"));
	}

	bOpInProgress = false;
}

void UAO_OnlineSessionSubsystem::ShowInviteUI()
{
	const IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
	if (OSS == nullptr)
	{
		AO_LOG(LogJSH, Warning, TEXT("ShowInviteUI: OSS null"));
		return;
	}

	IOnlineExternalUIPtr ExternalUI = OSS->GetExternalUIInterface();
	if (!ExternalUI.IsValid())
	{
		AO_LOG(LogJSH, Warning, TEXT("ShowInviteUI: ExternalUI invalid"));
		return;
	}
	
	const int32 LocalUserNum = 0;
	ExternalUI->ShowInviteUI(LocalUserNum, NAME_GameSession);
}

/* ==================== Destroy (호스트/클라 분리) ==================== */
void UAO_OnlineSessionSubsystem::DestroyCurrentSession()
{
	if (bOpInProgress)
	{
		AO_LOG(LogJSH, Warning, TEXT("DestroyCurrentSession: Operation in progress, request ignored"));
		return;
	}
	bOpInProgress = true;

	IOnlineSessionPtr Session = GetSessionInterface();
	if (!Session.IsValid())
	{
		AO_LOG(LogJSH, Warning, TEXT("DestroyCurrentSession: Session interface invalid, fallback to main menu"));
		bOpInProgress = false;
		if (UWorld* World = GetWorld())
		{
			UGameplayStatics::OpenLevel(World, GetMainMenuMapName(), true);
		}
		else
		{
			AO_LOG(LogJSH, Warning, TEXT("DestroyCurrentSession: World is null, cannot travel to main menu"));
		}
		return;
	}

	// JM : 세션 이탈시 보이스 채팅 나가기
	StopVoiceChat();

	const bool bIsHost = IsLocalHost();

	if (bIsHost)
	{
		/* 호스트: 세션 파괴 완료 콜백에서 메인 메뉴 복귀 */
		bPendingReturnToMenu = true;

		if (DestroyHandle.IsValid())
		{
			Session->ClearOnDestroySessionCompleteDelegate_Handle(DestroyHandle);
			DestroyHandle.Reset();
		}
		DestroyHandle = Session->AddOnDestroySessionCompleteDelegate_Handle(
			FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroyThenRecreateSession));

		if (!Session->DestroySession(NAME_GameSession))
		{
			AO_LOG(LogJSH, Warning, TEXT("DestroyCurrentSession: DestroySession failed (Host path)"));
			Session->ClearOnDestroySessionCompleteDelegate_Handle(DestroyHandle);
			DestroyHandle.Reset();
			bPendingReturnToMenu = false;
			bOpInProgress = false;
		}
		else
		{
			AO_LOG(LogJSH, Log, TEXT("DestroyCurrentSession: DestroySession requested (Host path)"));
		}
	}
	else
	{
		/* 클라이언트: 로컬 세션 파괴 요청 후 즉시 메인 메뉴로 이동 (콜백 기다리지 않음) */
		AO_LOG(LogJSH, Log, TEXT("DestroyCurrentSession: DestroySession requested (Client path)"));
		Session->DestroySession(NAME_GameSession);
		bOpInProgress = false;

		if (UWorld* World = GetWorld())
		{
			UGameplayStatics::OpenLevel(World, GetMainMenuMapName(), true);
		}
		else
		{
			AO_LOG(LogJSH, Warning, TEXT("DestroyCurrentSession: World is null, cannot travel to main menu (Client path)"));
		}
	}
}

void UAO_OnlineSessionSubsystem::OnDestroyThenRecreateSession(FName SessionName, bool bWasSuccessful)
{
	if (IOnlineSessionPtr Session = GetSessionInterface(); Session.IsValid())
	{
		if (DestroyHandle.IsValid())
		{
			Session->ClearOnDestroySessionCompleteDelegate_Handle(DestroyHandle);
			DestroyHandle.Reset();
		}
	}
	else
	{
		AO_LOG(LogJSH, Warning, TEXT("OnDestroyThenRecreateSession: Session interface invalid when clearing delegate"));
	}
	
	AO_LOG(LogJSH, Log, TEXT("OnDestroyThenRecreateSession: SessionName=%s, Success=%d, PendingReturnToMenu=%d, PendingRehost=%d, PendingInviteJoin=%d"),
		*SessionName.ToString(),
		static_cast<int32>(bWasSuccessful),
		static_cast<int32>(bPendingReturnToMenu),
		static_cast<int32>(bPendingRehost),
		static_cast<int32>(bPendingInviteJoin));

	if (bWasSuccessful && bPendingReturnToMenu)
	{
		bPendingReturnToMenu = false;
		bOpInProgress = false;

		if (UWorld* World = GetWorld())
		{
			UGameplayStatics::OpenLevel(World, GetMainMenuMapName(), true);
		}
		else
		{
			AO_LOG(LogJSH, Warning, TEXT("OnDestroyThenRecreateSession: World is null, cannot travel to main menu"));
		}
		return;
	}

	/* 1) 재호스트 */
	if (bWasSuccessful && bPendingRehost)
	{
		bPendingRehost = false;
		const int32 ReNum = CachedNumPublicConnections;
		const bool ReLAN = bCachedIsLAN;
		const FString ReName = CachedRoomName;
		const FString RePw = CachedPassword;

		AO_LOG(LogJSH, Log, TEXT("OnDestroyThenRecreateSession: Rehost path (Num=%d, LAN=%d, Room=%s)"),
			ReNum,
			static_cast<int32>(ReLAN),
			*ReName);

		bOpInProgress = false;
		HostSessionEx(ReNum, ReLAN, ReName, RePw);
		return;
	}
	/* 2) 초대 합류 */
	else if (bWasSuccessful && bPendingInviteJoin)
	{
		bPendingInviteJoin = false;

		if (IOnlineSessionPtr S = GetSessionInterface(); S.IsValid())
		{
			AO_LOG(LogJSH, Log, TEXT("OnDestroyThenRecreateSession: InviteJoin path"));

			if (JoinHandle.IsValid())
			{
				S->ClearOnJoinSessionCompleteDelegate_Handle(JoinHandle);
				JoinHandle.Reset();
			}
			JoinHandle = S->AddOnJoinSessionCompleteDelegate_Handle(
				FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete));

			if (!S->JoinSession(0, NAME_GameSession, CachedInviteResult))
			{
				AO_LOG(LogJSH, Warning, TEXT("OnDestroyThenRecreateSession: JoinSession failed on InviteJoin path"));
				S->ClearOnJoinSessionCompleteDelegate_Handle(JoinHandle);
				JoinHandle.Reset();
				bOpInProgress = false;
			}
			return;
		}
		else
		{
			AO_LOG(LogJSH, Warning, TEXT("OnDestroyThenRecreateSession: Session interface invalid on InviteJoin path"));
		}
	}

	if (!bWasSuccessful)
	{
		AO_LOG(LogJSH, Warning, TEXT("OnDestroyThenRecreateSession: DestroySession was not successful and no pending actions handled"));
	}
	bOpInProgress = false;
}

/* ==================== 조회/검증 ==================== */
FString UAO_OnlineSessionSubsystem::GetSessionOwnerNameByIndex(int32 Index) const
{
	return (Index >= 0 && Index < LastSearchResults.Num()) ? LastSearchResults[Index].Session.OwningUserName : FString();
}

int32 UAO_OnlineSessionSubsystem::GetOpenPublicConnectionsByIndex(int32 Index) const
{
	if (Index < 0 || Index >= LastSearchResults.Num())
	{
		return 0;
	}
	return LastSearchResults[Index].Session.NumOpenPublicConnections;
}

int32 UAO_OnlineSessionSubsystem::GetMaxPublicConnectionsByIndex(int32 Index) const
{
	if (Index < 0 || Index >= LastSearchResults.Num())
	{
		return 0;
	}
	return LastSearchResults[Index].Session.SessionSettings.NumPublicConnections;
}

FString UAO_OnlineSessionSubsystem::GetServerNameByIndex(int32 Index) const
{
	if (Index < 0 || Index >= LastSearchResults.Num())
	{
		return FString();
	}
	FString Name;
	LastSearchResults[Index].Session.SessionSettings.Get(KEY_SERVER_NAME, Name);
	return Name;
}

bool UAO_OnlineSessionSubsystem::IsPasswordRequiredByIndex(int32 Index) const
{
	if (Index < 0 || Index >= LastSearchResults.Num())
	{
		return false;
	}
	bool bHasPw = false;
	LastSearchResults[Index].Session.SessionSettings.Get(KEY_HAS_PASSWORD, bHasPw);
	return bHasPw;
}

bool UAO_OnlineSessionSubsystem::VerifyPasswordAgainstIndex(int32 Index, const FString& PlainPassword) const
{
	if (Index < 0 || Index >= LastSearchResults.Num())
	{
		return false;
	}

	FString SavedHash;
	if (!LastSearchResults[Index].Session.SessionSettings.Get(KEY_PASSWORD_MD5, SavedHash))
	{
		return PlainPassword.IsEmpty();
	}
	return SavedHash.Equals(ToMD5(PlainPassword), ESearchCase::IgnoreCase);
}

/* ==================== 초대 ==================== */
void UAO_OnlineSessionSubsystem::OnSessionUserInviteAccepted(
	bool bWasSuccessful, int32 LocalUserNum,
	TSharedPtr<const FUniqueNetId> UserId,
	const FOnlineSessionSearchResult& InviteResult)
{
	if (bOpInProgress)
	{
		AO_LOG(LogJSH, Warning, TEXT("OnSessionUserInviteAccepted: Operation in progress, invite ignored"));
		return;
	}
	bOpInProgress = true;

	AO_LOG(LogJSH, Log, TEXT("OnSessionUserInviteAccepted: Success=%d, LocalUserNum=%d"),
		static_cast<int32>(bWasSuccessful),
		LocalUserNum);

	if (!bWasSuccessful)
	{
		AO_LOG(LogJSH, Warning, TEXT("OnSessionUserInviteAccepted: bWasSuccessful is false"));
		bOpInProgress = false;
		return;
	}

	IOnlineSessionPtr Session = GetSessionInterface();
	if (!Session.IsValid())
	{
		AO_LOG(LogJSH, Warning, TEXT("OnSessionUserInviteAccepted: Session interface invalid"));
		bOpInProgress = false;
		return;
	}

	if (Session->GetNamedSession(NAME_GameSession) != nullptr)
	{
		AO_LOG(LogJSH, Log, TEXT("OnSessionUserInviteAccepted: Already in session, will destroy then join invite"));

		bPendingInviteJoin = true;
		CachedInviteResult = InviteResult;
		bPendingRehost = false;

		if (DestroyHandle.IsValid())
		{
			Session->ClearOnDestroySessionCompleteDelegate_Handle(DestroyHandle);
			DestroyHandle.Reset();
		}
		DestroyHandle = Session->AddOnDestroySessionCompleteDelegate_Handle(
			FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroyThenRecreateSession));

		if (!Session->DestroySession(NAME_GameSession))
		{
			AO_LOG(LogJSH, Warning, TEXT("OnSessionUserInviteAccepted: DestroySession failed while processing invite"));
			Session->ClearOnDestroySessionCompleteDelegate_Handle(DestroyHandle);
			DestroyHandle.Reset();
			bOpInProgress = false;
		}
		return;
	}

	if (JoinHandle.IsValid())
	{
		Session->ClearOnJoinSessionCompleteDelegate_Handle(JoinHandle);
		JoinHandle.Reset();
	}
	JoinHandle = Session->AddOnJoinSessionCompleteDelegate_Handle(
		FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete));

	if (!Session->JoinSession(LocalUserNum, NAME_GameSession, InviteResult))
	{
		AO_LOG(LogJSH, Warning, TEXT("OnSessionUserInviteAccepted: JoinSession failed (LocalUserNum=%d)"), LocalUserNum);
		Session->ClearOnJoinSessionCompleteDelegate_Handle(JoinHandle);
		JoinHandle.Reset();
		bOpInProgress = false;
		return;
	}
}

/* ==================== 자동 분기 API ==================== */
void UAO_OnlineSessionSubsystem::HostSessionAuto(int32 NumPublicConnections, const FString& RoomName, const FString& Password)
{
	const bool bLAN = IsNullOSS(); // NULL이면 LAN, Steam이면 Online
	AO_LOG(LogJSH, Log, TEXT("HostSessionAuto: Auto-selected mode (LAN=%d, Room=%s)"),
		static_cast<int32>(bLAN),
		*RoomName);
	HostSessionEx(NumPublicConnections, bLAN, RoomName, Password);
}

void UAO_OnlineSessionSubsystem::FindSessionsAuto(int32 MaxResults)
{
	const bool bLAN = IsNullOSS();
	AO_LOG(LogJSH, Log, TEXT("FindSessionsAuto: Auto-selected mode (LAN=%d, MaxResults=%d)"),
		static_cast<int32>(bLAN),
		MaxResults);
	FindSessions(MaxResults, bLAN);
}

/* ==================== Voice Chat (JM) ================ */
void UAO_OnlineSessionSubsystem::StartVoiceChat()
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	IOnlineVoicePtr VoiceInterface = GetOnlineVoiceInterface();
	if (!VoiceInterface.IsValid())
	{
		AO_LOG(LogJM, Warning, TEXT("Voice Interface is not Valid"));
		return;
	}
	// VoiceInterface->Init();		// TODO: 이번에 실험적으로 추가해봄
	VoiceInterface->RegisterLocalTalker(0);
	VoiceInterface->StartNetworkedVoice(0);
	bIsEnableVoiceChat = true;
	
	AO_LOG(LogJM, Log, TEXT("End"));
}

void UAO_OnlineSessionSubsystem::StopVoiceChat()
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	IOnlineVoicePtr VoiceInterface = GetOnlineVoiceInterface();
	if (!VoiceInterface.IsValid())
	{
		AO_LOG(LogJM, Warning, TEXT("Voice Interface is not Valid"));
		return;
	}

	VoiceInterface->ClearVoicePackets();			// 권장사항(추가됨)
	VoiceInterface->StopNetworkedVoice(0);
	VoiceInterface->RemoveAllRemoteTalkers();	// 이거 추가하니까 크래시 안남
	VoiceInterface->DisconnectAllEndpoints();	// 이거 추가하니까 크래시 안남
	VoiceInterface->UnregisterLocalTalker(0);	// 위의 과정 하고오니까 크래시 안남. 만약 크래시 나면 아래 타이머 다시 살리기
	bIsEnableVoiceChat = false;

	/* Unregister를 0.2초 뒤에 해서 정리될 시간을 줌 (필요시 추가) */
	/* TWeakObjectPtr<UAO_OnlineSessionSubsystem> WeakThis(this);
	if (UWorld* World = GetWorld())
	{
		FTimerHandle DelayHandle;
		World->GetTimerManager().SetTimer(
			DelayHandle,
			[WeakThis]()
			{
				if (!WeakThis.IsValid())
				{
					return;
				}
				if (WeakThis->GetOnlineVoiceInterface().IsValid())
				{
					WeakThis.Pin()->GetOnlineVoiceInterface()->UnregisterLocalTalker(0);
					AO_LOG(LogJM, Log, TEXT("Do Unregister Local Talker"));
				}
			},
			0.2f,
			false
		);
	}
	else
	{
		AO_LOG(LogJM, Warning, TEXT("No World, but try to unregister local talker"));
		VoiceInterface->UnregisterLocalTalker(0);
	}
	*/
	
	AO_LOG(LogJM, Log, TEXT("End"));
}

void UAO_OnlineSessionSubsystem::MuteRemoteTalker(const uint8 LocalUserNum, AAO_PlayerState* TargetPS, const bool bIsSystemWide)
{
	AO_LOG(LogJM, Log, TEXT("Start"));

	if (!TargetPS)
	{
		AO_LOG(LogJM, Warning, TEXT("Target PS is Null"));
		return;
	}
	
	TSharedPtr<const FUniqueNetId> TargetPSId = TargetPS->GetUniqueId().GetUniqueNetId();
	if (!TargetPSId.IsValid())
	{
		AO_LOG(LogJM, Warning, TEXT("TargetPSId is Not Valid"));
		return;
	}
	
	IOnlineVoicePtr VoiceInterface = GetOnlineVoiceInterface();
	if (!VoiceInterface.IsValid())
	{
		AO_LOG(LogJM, Warning, TEXT("InValid Voice Interface"));
		return;
	}

	if (VoiceInterface->MuteRemoteTalker(LocalUserNum, *TargetPSId, bIsSystemWide))
	{
		AO_LOG(LogJM, Log, TEXT("PS(%s) Muted"), *TargetPS->GetPlayerName());
	}
	else
	{
		// 호스트의 경우 Register가 안되어있는 문제가 있음 (Register Remote Talker 후, Mute 시도)
		AO_LOG(LogJM, Warning, TEXT("Mute Failed. Try RegisterRemoteTalker & Mute Again"));
		VoiceInterface->RegisterRemoteTalker(*TargetPSId);
		if (VoiceInterface->MuteRemoteTalker(LocalUserNum, *TargetPSId, bIsSystemWide))
		{
			AO_LOG(LogJM, Log, TEXT("PS(%s) Registered Remote Talker and Muted Successfully"), *TargetPS->GetPlayerName());
		}
		else
		{
			AO_LOG(LogJM, Error, TEXT("PS(%s), Finally Mute Failed even after Register Remote Talker"), *TargetPS->GetPlayerName());
		}
	}
	
	AO_LOG(LogJM, Log, TEXT("End"));
}

void UAO_OnlineSessionSubsystem::UnmuteRemoteTalker(const uint8 LocalUserNum, AAO_PlayerState* TargetPS, const bool bIsSystemWide)
{
	AO_LOG(LogJM, Log, TEXT("Start"));

	if (!TargetPS)
	{
		AO_LOG(LogJM, Warning, TEXT("Target PS is Null"));
		return;
	}
	
	TSharedPtr<const FUniqueNetId> TargetPSId = TargetPS->GetUniqueId().GetUniqueNetId();
	if (!TargetPSId.IsValid())
	{
		AO_LOG(LogJM, Warning, TEXT("TargetPSId is Not Valid"));
		return;
	}
	
	IOnlineVoicePtr VoiceInterface = GetOnlineVoiceInterface();
	if (!VoiceInterface.IsValid())
	{
		AO_LOG(LogJM, Warning, TEXT("InValid Voice Interface"));
		return;
	}

	if (VoiceInterface->UnmuteRemoteTalker(LocalUserNum, *TargetPSId, bIsSystemWide))
	{
		AO_LOG(LogJM, Log, TEXT("PS(%s) Unmuted"), *TargetPS->GetPlayerName());
	}
	else
	{
		// 호스트의 경우 Register가 안되어있는 문제가 있음 (Register Remote Talker 후, Unmute 시도)
		AO_LOG(LogJM, Warning, TEXT("Unmute Failed. Try RegisterRemoteTalker & Unmute Again"));
		VoiceInterface->RegisterRemoteTalker(*TargetPSId);
	}
	
	AO_LOG(LogJM, Log, TEXT("End"));
}
