// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "AO_GameState.generated.h"

/**
 * 
 */
UCLASS()
class AO_API AAO_GameState : public AGameState
{
	GENERATED_BODY()
	
public:
	AAO_GameState();
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void AddPlayerState(APlayerState* PlayerState) override; // JM : 플레이어 입장 시(레벨 이동 후 들어올 때) 해당 플레이어 unmute 하기

public:
	void SetSharedReviveCount(int32 InValue);

	UFUNCTION()
	void UnmuteVoiceOnAddPlayerState(APlayerState* PlayerState);	// JM : 플레이어 입장하면 해당 플레이어를 언뮤트 시킴

	UFUNCTION(BlueprintCallable, Category = "AO|Revive")
	int32 GetSharedReviveCount() const;

protected:
	UFUNCTION()
	void OnRep_SharedReviveCount();
	
public:
	UPROPERTY(ReplicatedUsing = OnRep_SharedReviveCount, VisibleAnywhere, BlueprintReadOnly, Category = "AO|Revive")
	int32 SharedReviveCount;

protected:
	FTimerHandle UnmuteVoiceTimerHandle;

//ms: 패시브 초기화
protected:
	UPROPERTY(ReplicatedUsing = OnRep_RunResetTrigger)
	int32 RunResetTrigger = 0;

	UFUNCTION()
	void OnRep_RunResetTrigger();

public:
	void Authority_NotifyGlobalReset();
	
	//ms : 선발대 흔적 확인
	UPROPERTY(Replicated)
	bool bHint1 = false;
	UPROPERTY(Replicated)
	bool bHint2 = false;
	UPROPERTY(Replicated)
	bool bHint3 = false;

	UFUNCTION(BlueprintCallable)
	void FindHint(int32 Num);
	bool CheckHintCount();
	//-ms
};
