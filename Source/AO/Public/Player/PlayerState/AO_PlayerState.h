// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AO_PlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAOLobbyReadyChanged, bool, bNewReady);

UCLASS()
class AO_API AAO_PlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AAO_PlayerState();

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_PlayerName() override;

	// JM : 생명주기 테스트용
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/* ==================== 로비 레디 상태 ==================== */

	// 로비 Ready 플래그 설정
	void SetLobbyReady(bool bNewReady);
	bool IsLobbyReady() const;

	// Ready 플래그 변경 브로드캐스트 (UI 바인딩용)
	UPROPERTY(BlueprintAssignable)
	FAOLobbyReadyChanged OnLobbyReadyChanged;

	/* ==================== 로비 입장 순서 / 호스트 ==================== */

	// 로비 입장 순서 설정 (서버 전용)
	void SetLobbyJoinOrder(int32 InOrder);

	// 로비 입장 순서 조회
	int32 GetLobbyJoinOrder() const;

	// 호스트 플래그 설정 (서버 전용)
	void SetIsLobbyHost(bool bNewIsHost);

	// 호스트 여부 (입장 순서 0번을 호스트로 간주)
	bool IsLobbyHost() const;

	// 레디 상태가 복제될 때 호출
	UFUNCTION()
	void OnRep_LobbyIsReady();

protected:
	// 레디 상태 (변경 시 OnRep_LobbyIsReady 호출)
	UPROPERTY(ReplicatedUsing=OnRep_LobbyIsReady)
	bool bLobbyIsReady;

	// 로비 입장 순서 (0부터 시작, -1이면 아직 미할당)
	UPROPERTY(ReplicatedUsing=OnRep_LobbyJoinOrder)
	int32 LobbyJoinOrder;

	// 호스트 여부 (세션 동안 유지, GameInstance 기반)
	UPROPERTY(ReplicatedUsing=OnRep_IsLobbyHost)
	bool bIsLobbyHost;
	
	// 입장 순서가 복제될 때 호출
	UFUNCTION()
	void OnRep_LobbyJoinOrder();

	// 호스트 플래그가 복제될 때 호출
	UFUNCTION()
	void OnRep_IsLobbyHost();

private:
	// 현재 월드의 모든 LobbyReadyBoardActor에 보드 재빌드 요청
	void RefreshLobbyReadyBoard();

// JM : 생존 여부 판단용 변수 추가 (임시)
public:
	bool bIsAlive = true;
};
