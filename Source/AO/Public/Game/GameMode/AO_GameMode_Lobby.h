// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "AO_GameMode_Lobby.generated.h"

/**
 * 
 */
UCLASS()
class AO_API AAO_GameMode_Lobby : public AGameMode
{
	GENERATED_BODY()

public:
	AAO_GameMode_Lobby();

public:
	void PostLogin(APlayerController* NewPlayer) override;
	void Logout(AController* Exiting) override;
	void HandleSeamlessTravelPlayer(AController*& C) override;		// JM : 레벨 이동시 Voice Chat 자동 활성화 

public:
	/* PC가 레디/레디해제 했을 때 호출 (서버) */
	void SetPlayerReady(AController* Controller, bool bReady);

	/* PC가 시작을 요청했을 때 호출 (서버) */
	void RequestStartFrom(AController* Controller);

	/* 상태 변경 감지 → 레디 보드에 반영 */
	void NotifyLobbyBoardChanged();

	/* 레벨 이동 전 voice chat 전부 비활성화 */
	void StopVoiceChatForAllClients();

protected:
	/* 호스트를 제외한 플레이어 중 레디한 컨트롤러 목록 */
	UPROPERTY()
	TSet<TObjectPtr<AController>> ReadyPlayers;

	/* 현재 호스트 컨트롤러(입장 순서 0번 기준) */
	AController* GetHostController() const;

	/* 호스트를 제외한 모두가 레디인지 확인 */
	bool IsEveryoneReadyExceptHost() const;

	/* 실제 스테이지로 이동 처리 */
	void TravelToStage();

	/* ========== 로비 입장 순서 관리 ========== */

protected:
	// 다음에 입장하는 플레이어에게 부여할 순번
	int32 NextLobbyJoinOrder;

	// 현재 PlayerState 들을 보고 NextLobbyJoinOrder 재계산
	void UpdateNextLobbyJoinOrderFromExistingPlayers();

	// 아직 순번을 못 받은 PlayerState 에 순번 부여
	void AssignJoinOrderIfNeeded(APlayerState* PlayerState);
};
