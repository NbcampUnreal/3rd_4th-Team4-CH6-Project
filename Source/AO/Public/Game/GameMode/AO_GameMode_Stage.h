// AO_GameMode_Stage.h (장주만)

#pragma once

#include "CoreMinimal.h"
#include "AO_GameMode_InGameBase.h"
#include "AO_GameMode_Stage.generated.h"

/**
 * 
 */
UCLASS()
class AO_API AAO_GameMode_Stage : public AAO_GameMode_InGameBase
{
	GENERATED_BODY()

public:
	AAO_GameMode_Stage();

public:
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual void HandleSeamlessTravelPlayer(AController*& C) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	/* 스테이지 출발 상호작용 시 서버에서 호출할 함수 */
	void HandleStageExitRequest(AController* Requester);

	/* JM수정 : 게임 실패 하면 위젯만 띄우도록 */
	void HandleStageFail(AController* Requester);

	/* JM : 실제 게임 실패 후 로비로 이동 */
	UFUNCTION(BlueprintCallable, Category = "AO|GameMode|Stage")
	void ResetGameAndGoToLobby(AController* Requester);
	
	/* 부활 카운트 증가 */
	void HandleSharedReviveCountIncreased();
			
	/* 기차 연료 조건 미달시 실패 트리거 */
	void TriggerStageFailByTrainFuel();
	
	/* 플레이어 생존 여부가 변경되었을 때 호출되는 함수 */
	void NotifyPlayerAliveStateChanged(class AAO_PlayerState* ChangedPlayerState);
	
	/* 공유 부활 카운트를 사용해서 한 플레이어를 시작 지점에서 부활(추후에 부활 어빌리티로 대체) */
	bool TryRevivePlayer(class APlayerController* ReviveTargetPC);
	
protected:
	/* 아직 살아 있는 플레이어가 한 명이라도 있는지 여부 */
	bool HasAnyAlivePlayer() const;

	/* 공용 부활 횟수와 생존자 수를 보고 전멸 여부 평가 */
	void EvaluateTeamWipe();
	
	/* 죽은 순서대로 자동 부활시키기 위한 대기 큐 */
	UPROPERTY()
	TArray<TWeakObjectPtr<AAO_PlayerState>> PendingReviveQueue;

	/* 부활 큐에 플레이어 추가 */
	void EnqueuePendingRevive(AAO_PlayerState* DeadPlayerState);

	/* 부활 큐에서 플레이어 제거 */
	void RemoveFromPendingRevive(AAO_PlayerState* PlayerState);

	/* 공용 부활 카운트와 큐 상태를 보고 자동 부활 처리 */
	void TryAutoReviveFromQueue();
	
	// 자동 부활 대기 시간(초)
	UPROPERTY(EditDefaultsOnly, Category = "Revive")
	float AutoReviveDelaySeconds;

	// 자동 부활용 타이머
	FTimerHandle AutoReviveTimerHandle;

	// 자동 부활 시도를 일정 시간 뒤에 호출하는 헬퍼
	void ScheduleAutoRevive();
	
	/* 방 진행정보 초기화 */
	void RollbackSessionInGameFlag();

protected:
	/* 스테이지 종료 여부 */
	bool bStageEnded = false;
	
	//ms
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Train")
	TObjectPtr<class UAO_FuelData> FuelDataAsset;

	UFUNCTION(BlueprintPure)
	float GetRuquireFuelValue();
};
