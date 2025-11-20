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

public:
	/* PC가 레디/레디해제 했을 때 호출 (서버) */
	void SetPlayerReady(AController* Controller, bool bReady);

	/* PC가 시작을 요청했을 때 호출 (서버) */
	void RequestStartFrom(AController* Controller);

	/* 상태 변경 감지 */
	void NotifyLobbyBoardChanged();

protected:
	/* 호스트를 제외한 플레이어 중 레디한 컨트롤러 목록 */
	UPROPERTY()
	TSet<TObjectPtr<AController>> ReadyPlayers;

	/* 현재 호스트 컨트롤러(PlayerArray[0] 기준) */
	AController* GetHostController() const;

	/* 호스트를 제외한 모두가 레디인지 확인 */
	bool IsEveryoneReadyExceptHost() const;

	/* 실제 스테이지로 이동 처리 */
	void TravelToStage();
};
