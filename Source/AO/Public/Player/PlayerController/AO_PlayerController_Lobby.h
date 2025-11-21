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
	virtual void SetupInputComponent() override;

protected:
	/** 클라이언트 측에서 자기 레디 상태를 기억하는 용도(디버그용) */
	UPROPERTY()
	bool bIsReady;

	/** 2키 입력 처리 (클라이언트) */
	void OnPressed_ReadyKey();

	/** 3키 입력 처리 (클라이언트) */
	void OnPressed_StartKey();

	/** 레디 상태를 서버에 전달 */
	UFUNCTION(Server, Reliable)
	void ServerSetReady(bool bNewReady);

	/** 시작 요청을 서버에 전달 */
	UFUNCTION(Server, Reliable)
	void ServerRequestStart();
};
