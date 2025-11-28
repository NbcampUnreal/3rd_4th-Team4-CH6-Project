// JSH: AO_GameInstance.h

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "AO_GameInstance.generated.h"

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

	// ===== 세션 단위 데이터 리셋 (메인메뉴로 나갈 때 사용) =====
	UFUNCTION(BlueprintCallable, Category="AO|Session")
	void ResetSessionData();

	// ===== 로비 호스트 관련 헬퍼 =====
	void ClearLobbyHostInfo();
	bool HasLobbyHost() const;
	void SetLobbyHostFromPlayerState(const class APlayerState* PlayerState);
	bool IsLobbyHostPlayerState(const class APlayerState* PlayerState) const;
};
