// JSH: AO_GameInstance.h

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "AO_GameInstance.generated.h"

class UAO_FuelData;

UCLASS()
class AO_API UAO_GameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UAO_GameInstance();

public:
	// 스테이지/휴게공간 사이를 오갈 때 유지할 연료 값 (서버 기준)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AO|Train")
	float SharedTrainFuel;

	// 현재 몇 번째 스테이지인지 (0 = Stage1, 1 = Stage2, 2 = Stage3 ...)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AO|Route")
	int32 CurrentStageIndex;

	// 세션 동안 호스트로 유지될 플레이어의 UniqueNetId 문자열
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AO|Lobby")
	FString LobbyHostNetIdStr;
	
	// 공유하는 부활 기본 횟수 (런 시작 시 한 번만 사용)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AO|Revive")
	int32 InitialSharedReviveCount;
	
	// 공유하는 부활 가능 횟수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AO|Revive")
	int32 SharedReviveCount;

	// JM : 게임 통계
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AO|Statistics")
	float GameStartTime;		// LV_Meadow_Main 레벨 블루프린트에서 Begin Play에서 기록 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AO|Statistics")
	float GameEndTime;			// GS::OnRep_Clear 에서 기록

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AO|Statistics")
	int32 TeamDeathCount;

public:
	// 이번 판을 처음부터 다시 시작 (스테이지 인덱스 / 연료 초기화)
	UFUNCTION(BlueprintCallable, Category="AO|Route")
	void ResetRun();

	// 현재 스테이지 맵 이름 (없으면 NAME_None)
	UFUNCTION(BlueprintCallable, Category="AO|Route")
	FName GetCurrentStageMap() const;

	// 휴게공간 맵 이름
	UFUNCTION(BlueprintCallable, Category="AO|Route")
	FName GetRestMap() const;

	// 로비 맵 이름
	UFUNCTION(BlueprintCallable, Category="AO|Route")
	FName GetLobbyMap() const;

	// 마지막 스테이지인지 여부
	UFUNCTION(BlueprintCallable, Category="AO|Route")
	bool IsLastStage() const;

	// 다음 스테이지로 인덱스를 증가 (성공 시 true, 더 이상 없으면 false)
	UFUNCTION(BlueprintCallable, Category="AO|Route")
	bool TryAdvanceStageIndex();

	// 세션 단위 데이터 리셋
	UFUNCTION(BlueprintCallable, Category="AO|Session")
	void ResetSessionData();

	/* ---------- 로비 호스트 관련 헬퍼 -------------- */
	void ClearLobbyHostInfo();
	bool HasLobbyHost() const;
	void SetLobbyHostFromPlayerState(const class APlayerState* PlayerState);
	bool IsLobbyHostPlayerState(const class APlayerState* PlayerState) const;
	
	/* ---------- 공유 부활 횟수 헬퍼 ----------- */
	// 현재 부활 가능 횟수 조회
	UFUNCTION(BlueprintCallable, Category="AO|Revive")
	int32 GetSharedReviveCount() const;

	//ms : 패시브 초기화
	void PassiveReset();

	//ms: 연료량 data asset
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Train")
	TObjectPtr<class UAO_FuelData> FuelDataAsset;

public:
	virtual void Init() override;

	UFUNCTION(BlueprintPure, Category = "Train")
	float GetInitialFuel();

	UFUNCTION(BlueprintPure)
	float GetInitialFuelValue();
};