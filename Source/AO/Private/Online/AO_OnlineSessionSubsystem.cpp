// Fill out your copyright notice in the Description page of Project Settings.

#include "Online/AO_OnlineSessionSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Online/OnlineSessionNames.h"
#include "Misc/SecureHash.h"

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
	return nullptr;
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
		return;
	}
	bOpInProgress = true;

	IOnlineSessionPtr Session = GetSessionInterface();
	if (!Session.IsValid())
	{
		bOpInProgress = false;
		return;
	}

	/* 기존 세션이면 제거 후 재호스트 */
	if (Session->GetNamedSession(NAME_GameSession) != nullptr)
	{
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
		Session->ClearOnCreateSessionCompleteDelegate_Handle(CreateHandle);
		CreateHandle.Reset();
		bOpInProgress = false;
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

	if (!bWasSuccessful)
	{
		bOpInProgress = false;
		return;
	}

	if (UWorld* World = GetWorld())
	{
		UGameplayStatics::OpenLevel(World, GetLobbyMapName(), true, TEXT("?listen"));
	}

	bOpInProgress = false;
}

/* ==================== Find ==================== */
void UAO_OnlineSessionSubsystem::FindSessions(int32 MaxResults, bool bIsLAN)
{
	/* 진행 중이면 먼저 취소 */
	if (bFinding)
	{
		CancelFind();
	}

	IOnlineSessionPtr Session = GetSessionInterface();
	if (!Session.IsValid())
	{
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
		Session->ClearOnFindSessionsCompleteDelegate_Handle(FindHandle);
		FindHandle.Reset();
		bFinding = false;
		OnFindSessionsCompleteEvent.Broadcast(false);
	}
}

void UAO_OnlineSessionSubsystem::CancelFind()
{
	IOnlineSessionPtr Session = GetSessionInterface();
	if (Session.IsValid())
	{
		Session->CancelFindSessions();
		if (FindHandle.IsValid())
		{
			Session->ClearOnFindSessionsCompleteDelegate_Handle(FindHandle);
			FindHandle.Reset();
		}
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

	bFinding = false;
	LastSearchResults.Reset();

	if (!bWasSuccessful || !LastSearch.IsValid())
	{
		OnFindSessionsCompleteEvent.Broadcast(false);
		return;
	}
	
	LastSearchResults = LastSearch->SearchResults;
	
	{
		TSet<FString> SeenIds;
		LastSearchResults.RemoveAll([&SeenIds](const FOnlineSessionSearchResult& R)
		{
			const FString Id = R.GetSessionIdStr();
			if(SeenIds.Contains(Id))
			{
				return true;
			}
			SeenIds.Add(Id);
			
			const auto& S = R.Session;
			bool bLobbyTag=false;
			S.SessionSettings.Get(FName(TEXT("LOBBYSEARCH")), bLobbyTag);

			FString RoomName;
			const bool bHasName = S.SessionSettings.Get(KEY_SERVER_NAME, RoomName);

			const bool bBad =
				!bLobbyTag ||
				!bHasName || RoomName.IsEmpty() ||
				S.OwningUserName.IsEmpty();

			return bBad;
		});
	}

	OnFindSessionsCompleteEvent.Broadcast(true);
}


/* ==================== Join ==================== */
void UAO_OnlineSessionSubsystem::JoinSessionByIndex(int32 Index)
{
	if (bOpInProgress)
	{
		return;
	}
	if (!LastSearch.IsValid() || Index < 0 || Index >= LastSearchResults.Num())
	{
		return;
	}
	bOpInProgress = true;

	IOnlineSessionPtr Session = GetSessionInterface();
	if (!Session.IsValid())
	{
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
		Session->ClearOnJoinSessionCompleteDelegate_Handle(JoinHandle);
		JoinHandle.Reset();
		bOpInProgress = false;
	}
}

void UAO_OnlineSessionSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSessionPtr Session = GetSessionInterface();
	if (!Session.IsValid())
	{
		bOpInProgress = false;
		return;
	}

	if (JoinHandle.IsValid())
	{
		Session->ClearOnJoinSessionCompleteDelegate_Handle(JoinHandle);
		JoinHandle.Reset();
	}

	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		bOpInProgress = false;
		return;
	}

	FString ConnectString;
	if (!Session->GetResolvedConnectString(SessionName, ConnectString))
	{
		bOpInProgress = false;
		return;
	}

	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		PC->ClientTravel(ConnectString, TRAVEL_Absolute);
	}

	bOpInProgress = false;
}

/* ==================== Destroy ==================== */
void UAO_OnlineSessionSubsystem::DestroyCurrentSession()
{
	if (bOpInProgress)
	{
		return;
	}
	bOpInProgress = true;

	IOnlineSessionPtr Session = GetSessionInterface();
	if (!Session.IsValid())
	{
		bOpInProgress = false;
		return;
	}

	if (DestroyHandle.IsValid())
	{
		Session->ClearOnDestroySessionCompleteDelegate_Handle(DestroyHandle);
		DestroyHandle.Reset();
	}
	DestroyHandle = Session->AddOnDestroySessionCompleteDelegate_Handle(
		FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroyThenRecreateSession));

	if (!Session->DestroySession(NAME_GameSession))
	{
		Session->ClearOnDestroySessionCompleteDelegate_Handle(DestroyHandle);
		DestroyHandle.Reset();
		bOpInProgress = false;
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

	if (bWasSuccessful && bPendingRehost)
	{
		bPendingRehost = false;
		const int32 ReNum = CachedNumPublicConnections;
		const bool ReLAN = bCachedIsLAN;
		const FString ReName = CachedRoomName;
		const FString RePw = CachedPassword;

		bOpInProgress = false;
		HostSessionEx(ReNum, ReLAN, ReName, RePw);
		return;
	}
	else if (bWasSuccessful && bPendingInviteJoin)
	{
		bPendingInviteJoin = false;

		if (IOnlineSessionPtr S = GetSessionInterface(); S.IsValid())
		{
			if (JoinHandle.IsValid())
			{
				S->ClearOnJoinSessionCompleteDelegate_Handle(JoinHandle);
				JoinHandle.Reset();
			}
			JoinHandle = S->AddOnJoinSessionCompleteDelegate_Handle(
				FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete));

			if (!S->JoinSession(0, NAME_GameSession, CachedInviteResult))
			{
				S->ClearOnJoinSessionCompleteDelegate_Handle(JoinHandle);
				JoinHandle.Reset();
				bOpInProgress = false;
			}
			return;
		}
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
		return;
	}
	bOpInProgress = true;

	if (!bWasSuccessful)
	{
		bOpInProgress = false;
		return;
	}

	IOnlineSessionPtr Session = GetSessionInterface();
	if (!Session.IsValid())
	{
		bOpInProgress = false;
		return;
	}

	if (Session->GetNamedSession(NAME_GameSession) != nullptr)
	{
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
	HostSessionEx(NumPublicConnections, bLAN, RoomName, Password);
}

void UAO_OnlineSessionSubsystem::FindSessionsAuto(int32 MaxResults)
{
	const bool bLAN = IsNullOSS();
	FindSessions(MaxResults, bLAN);
}
