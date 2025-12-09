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

	/* 게임 실패 처리 */
	void HandleStageFail(AController* Requester);
			
	/* 기차 연료 조건 미달시 실패 트리거 */
	void TriggerStageFailByTrainFuel();
	
	/* 플레이어 생존 여부가 변경되었을 때 호출되는 함수 */
	void NotifyPlayerAliveStateChanged(class AAO_PlayerState* ChangedPlayerState);
	
protected:
	/* 아직 살아 있는 플레이어가 한 명이라도 있는지 여부 */
	bool HasAnyAlivePlayer() const;

	/* 공용 부활 횟수와 생존자 수를 보고 전멸 여부 평가 */
	void EvaluateTeamWipe();
	
protected:
	/* 열차(연료) 정보 받아오기 */
	void SaveTrainFuelToGameInstance();
};
