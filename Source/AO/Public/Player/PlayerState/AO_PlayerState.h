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
	
	// 로비 Ready 플래그 설정
	void SetLobbyReady(bool bNewReady);
	bool IsLobbyReady() const;

	UPROPERTY(BlueprintAssignable)
	FAOLobbyReadyChanged OnLobbyReadyChanged;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_LobbyIsReady)
	bool bLobbyIsReady;

	UFUNCTION()
	void OnRep_LobbyIsReady();

private:
	// 보드 재빌드
	void RefreshLobbyReadyBoard();

// JM : 생존 여부 판단용 변수 추가 (임시)
public:
	bool bIsAlive = true;
};
