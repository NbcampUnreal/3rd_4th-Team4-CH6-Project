// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AO_PlayerController_InGameBase.h"
#include "AO_PlayerController_Lobby.generated.h"

/**
 * 
 */
UCLASS()
class AO_API AAO_PlayerController_Lobby : public AAO_PlayerController_InGameBase
{
	GENERATED_BODY()

public:
	AAO_PlayerController_Lobby();

protected:
	virtual void BeginPlay() override;

public:
	/* 스팀 초대 오버레이 UI 열기 */
	UFUNCTION(Client, Reliable)
	void Client_OpenInviteOverlay();

	/* 레디 상태를 서버에 전달 */
	UFUNCTION(Server, Reliable)
	void Server_SetReady(bool bNewReady);

	/* 시작 요청을 서버에 전달 */
	UFUNCTION(Server, Reliable)
	void Server_RequestStart();

};
