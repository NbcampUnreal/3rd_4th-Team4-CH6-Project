// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "Player/PlayerState/AO_PlayerState.h"
#include "AO_OnlineSessionSubsystem.generated.h"

namespace AO_SessionKeys
{
	static const FName KEY_SERVER_NAME(TEXT("SERVER_NAME"));
	static const FName KEY_HAS_PASSWORD(TEXT("HAS_PASSWORD"));
	static const FName KEY_PASSWORD_MD5(TEXT("PASSWORD_MD5"));
}

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAO_OnFindSessionsComplete, bool, bSuccessful);

UCLASS()
class AO_API UAO_OnlineSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UAO_OnlineSessionSubsystem();

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/* ==================== Blueprint API ==================== */
	UFUNCTION(BlueprintCallable, Category="AO|Online")
	void HostSession(int32 NumPublicConnections, bool bIsLAN);

	UFUNCTION(BlueprintCallable, Category="AO|Online")
	void HostSessionEx(int32 NumPublicConnections, bool bIsLAN, const FString& RoomName, const FString& Password);

	UFUNCTION(BlueprintCallable, Category="AO|Online")
	void FindSessions(int32 MaxResults, bool bIsLAN);

	UFUNCTION(BlueprintCallable, Category="AO|Online")
	void CancelFind();

	UFUNCTION(BlueprintCallable, Category="AO|Online")
	void JoinSessionByIndex(int32 Index);

	UFUNCTION(BlueprintCallable, Category="AO|Online")
	void DestroyCurrentSession();

	/* ==================== 조회용 ==================== */
	UFUNCTION(BlueprintPure, Category="AO|Online")
	int32 GetNumSearchResults() const { return LastSearchResults.Num();	}

	UFUNCTION(BlueprintPure, Category="AO|Online")
	FString GetSessionOwnerNameByIndex(int32 Index) const;

	UFUNCTION(BlueprintPure, Category="AO|Online")
	int32 GetOpenPublicConnectionsByIndex(int32 Index) const;

	UFUNCTION(BlueprintPure, Category="AO|Online")
	int32 GetMaxPublicConnectionsByIndex(int32 Index) const;

	UFUNCTION(BlueprintPure, Category="AO|Online")
	FString GetServerNameByIndex(int32 Index) const;

	UFUNCTION(BlueprintPure, Category="AO|Online")
	bool IsPasswordRequiredByIndex(int32 Index) const;

	UFUNCTION(BlueprintCallable, Category="AO|Online")
	bool VerifyPasswordAgainstIndex(int32 Index, const FString& PlainPassword) const;

	UFUNCTION(BlueprintPure, Category="AO|Online")
	bool IsFinding() const { return bFinding; }

	/* 세션 검색 완료 알림 (UI용) */
	UPROPERTY(BlueprintAssignable, Category="AO|Online")
	FAO_OnFindSessionsComplete OnFindSessionsCompleteEvent;

	/* 자동 분기 */
	UFUNCTION(BlueprintCallable, Category="AO|Online")
	void HostSessionAuto(int32 NumPublicConnections, const FString& RoomName = TEXT("New Room"), const FString& Password = TEXT(""));

	UFUNCTION(BlueprintCallable, Category="AO|Online")
	void FindSessionsAuto(int32 MaxResults = 50);

// Voice Chat (JM)
public:
	IOnlineVoicePtr GetOnlineVoiceInterface() const;	// JM
	
	UFUNCTION(BlueprintCallable, Category="AO|VoiceChat")
	void StartVoiceChat();

	UFUNCTION(BlueprintCallable, Category="AO|VoiceChat")
	void StopVoiceChat();

	UFUNCTION(BlueprintCallable, Category="AO|VoiceChat")
	void MuteRemoteTalker(const uint8 LocalUserNum, AAO_PlayerState* TargetPS, const bool bIsSystemWide);

	UFUNCTION(BlueprintCallable, Category="AO|VoiceChat")
	void UnmuteRemoteTalker(const uint8 LocalUserNum, AAO_PlayerState* TargetPS, const bool bIsSystemWide);

protected:
	IOnlineSessionPtr GetSessionInterface() const;

	/* 세션 델리게이트 */
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroyThenRecreateSession(FName SessionName, bool bWasSuccessful);

public:
	/* 스팀 초대 UI */
	void ShowInviteUI();
	
	/* 호스트/클라이언트 분기용 */
	bool IsLocalHost() const;
	
private:
	/* 검색 결과 캐시 */
	TSharedPtr<FOnlineSessionSearch> LastSearch;
	TArray<FOnlineSessionSearchResult> LastSearchResults;

	/* 델리게이트 핸들 */
	FDelegateHandle CreateHandle;
	FDelegateHandle FindHandle;
	FDelegateHandle JoinHandle;
	FDelegateHandle DestroyHandle;

	/* 재호스트 캐시 */
	int32 CachedNumPublicConnections = 4;
	bool bCachedIsLAN = false;
	bool bPendingRehost = false;
	FString CachedRoomName;
	FString CachedPassword;

	/* 스팀 초대 */
	void OnSessionUserInviteAccepted(bool bWasSuccessful, int32 LocalUserNum,
		TSharedPtr<const FUniqueNetId> UserId, const FOnlineSessionSearchResult& InviteResult);
	FDelegateHandle InviteAcceptedHandle;

	bool bPendingInviteJoin = false;
	FOnlineSessionSearchResult CachedInviteResult;

	/* 맵 이름 */
	static FName GetMainMenuMapName()
	{
		return FName(TEXT("/Game/AVaOut/Maps/LV_MainMenu"));
	}

	static FName GetLobbyMapName()
	{
		return FName(TEXT("/Game/AVaOut/Maps/LV_Lobby"));
	}
	
	static FString ToMD5(const FString& In);

	/* 네트워크 연결 실패 에러 */
	void HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);
	FDelegateHandle NetFailHandle;

private:
	/* ====== 최소 안정화 상태 ====== */
	bool bFinding = false;       // Find 진행 여부
	bool bOpInProgress = false;  // Host/Join/Destroy 작업 재진입 가드

	/* Destroy 후 동작 제어 */
	bool bPendingReturnToMenu = false; // 호스트: Destroy 완료 후 메인 메뉴 복귀

public:
	UPROPERTY(BlueprintReadWrite, Category="AO|VoiceChat")
	bool bIsEnableVoiceChat = true;
};
