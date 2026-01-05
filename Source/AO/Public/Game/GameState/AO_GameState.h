// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "AO_GameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSharedReviveCountChanged, int32, NewCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStageFailed);

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

	void SetStageFailed();
	
	UFUNCTION()
	void UnmuteVoiceOnAddPlayerState(APlayerState* PlayerState);	// JM : 플레이어 입장하면 해당 플레이어를 언뮤트 시킴

	UFUNCTION(BlueprintCallable, Category = "AO|Revive")
	int32 GetSharedReviveCount() const;

	UFUNCTION(BlueprintCallable, Category = "AO|GameFlow")
	bool IsStageFailed() const { return bIsStageFailed; }

protected:
	UFUNCTION()
	void OnRep_SharedReviveCount();

	UFUNCTION()
	void OnRep_IsStageFailed();
	
public:
	UPROPERTY(ReplicatedUsing = OnRep_SharedReviveCount, VisibleAnywhere, BlueprintReadOnly, Category = "AO|Revive")
	int32 SharedReviveCount;

	UPROPERTY(ReplicatedUsing = OnRep_IsStageFailed, VisibleAnywhere, BlueprintReadOnly, Category = "AO|GameFlow")
	bool bIsStageFailed;
	
	

public:
	UPROPERTY(BlueprintAssignable, Category = "AO|Revive")
	FOnSharedReviveCountChanged OnSharedReviveCountChanged;

	UPROPERTY(BlueprintAssignable, Category = "AO|GameFlow")
	FOnStageFailed OnStageFailed;

protected:
	FTimerHandle UnmuteVoiceTimerHandle;
	
};
