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
// TODO: JM 가상함수 -> 일반함수 -> 변수 순서대로 정렬
public:
	UPROPERTY(ReplicatedUsing = OnRep_SharedReviveCount, VisibleAnywhere, BlueprintReadOnly, Category = "AO|Revive")
	int32 SharedReviveCount;

protected:
	FTimerHandle UnmuteVoiceTimerHandle;

protected:
	UFUNCTION()
	void OnRep_SharedReviveCount();

public:
	void SetSharedReviveCount(int32 InValue);

	UFUNCTION()
	void UnmuteVoiceOnAddPlayerState(APlayerState* PlayerState);	// JM : 플레이어 입장하면 해당 플레이어를 언뮤트 시킴

	UFUNCTION(BlueprintCallable, Category = "AO|Revive")
	int32 GetSharedReviveCount() const;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void AddPlayerState(APlayerState* PlayerState) override; // JM : 플레이어 입장 시(레벨 이동 후 들어올 때) 해당 플레이어 unmute 하기 
	
};
