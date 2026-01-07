// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "AO_GameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSharedReviveCountChanged, int32, NewCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStageFailed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameCleared);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHintChangedSignature, int32, NewHintNum);

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
	virtual void BeginPlay() override;

public:
	void SetSharedReviveCount(int32 InValue);
	void SetStageFailed();
	void SetGameClear();
	void AddTeamDeathCount();		// JM : For Statistics
	void SetGameStartTime();
	void SetGameEndTime();
	
	UFUNCTION()
	void UnmuteVoiceOnAddPlayerState(APlayerState* PlayerState);	// JM : 플레이어 입장하면 해당 플레이어를 언뮤트 시킴

	UFUNCTION(BlueprintCallable, Category = "AO|Revive")
	int32 GetSharedReviveCount() const;

	UFUNCTION(BlueprintCallable, Category = "AO|Revive")
	void AddSharedReviveCount(int32 Delta);

	UFUNCTION(BlueprintCallable, Category = "AO|Revive")
	bool TryConsumeSharedReviveCount();

	UFUNCTION(BlueprintCallable, Category = "AO|GameFlow")
	bool IsStageFailed() const { return bIsStageFailed; }

	UFUNCTION(BlueprintCallable, Category = "AO|GameFlow")
	bool IsGameCleared() const { return bIsGameCleared; }

protected:
	UFUNCTION()
	void OnRep_SharedReviveCount();

	UFUNCTION()
	void OnRep_IsStageFailed();

	UFUNCTION()
	void OnRep_IsGameCleared();

	UFUNCTION()
	void OnRep_TeamDeathCount();

	UFUNCTION()
	void OnRep_GameStartTime();

	UFUNCTION()
	void OnRep_GameEndTime();
	
public:
	UPROPERTY(ReplicatedUsing = OnRep_SharedReviveCount, VisibleAnywhere, BlueprintReadOnly, Category = "AO|Revive")
	int32 SharedReviveCount;

	UPROPERTY(ReplicatedUsing = OnRep_IsStageFailed, VisibleAnywhere, BlueprintReadOnly, Category = "AO|GameFlow")
	bool bIsStageFailed;

	UPROPERTY(ReplicatedUsing = OnRep_IsGameCleared, VisibleAnywhere, BlueprintReadOnly, Category = "AO|GameFlow")
	bool bIsGameCleared;

	UPROPERTY(ReplicatedUsing = OnRep_TeamDeathCount, VisibleAnywhere, BlueprintReadOnly, Category = "AO|Statistics")
	int32 TeamDeathCount;

	UPROPERTY(ReplicatedUsing = OnRep_TeamDeathCount, VisibleAnywhere, BlueprintReadOnly, Category = "AO|Statistics")
	float GameStartTime;

	UPROPERTY(ReplicatedUsing = OnRep_TeamDeathCount, VisibleAnywhere, BlueprintReadOnly, Category = "AO|Statistics")
	float GameEndTime;

public:
	UPROPERTY(BlueprintAssignable, Category = "AO|Revive")
	FOnSharedReviveCountChanged OnSharedReviveCountChanged;

	UPROPERTY(BlueprintAssignable, Category = "AO|GameFlow")
	FOnStageFailed OnStageFailed;

	UPROPERTY(BlueprintAssignable, Category = "AO|GameFlow")
	FOnGameCleared OnGameCleared;

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
	bool bHint1;
	UPROPERTY(Replicated)
	bool bHint2;
	UPROPERTY(Replicated)
	bool bHint3;

	UFUNCTION(BlueprintCallable)
	void FindHint(int32 Num);
	UFUNCTION(BlueprintCallable)
	bool CheckHintCount();
	
	UPROPERTY(ReplicatedUsing = OnRep_HintCount, BlueprintReadWrite)
	int32 CurrentFindHintNum;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FHintChangedSignature OnHintCountChanged;
	UFUNCTION()
	void OnRep_HintCount();
	//-ms
};
