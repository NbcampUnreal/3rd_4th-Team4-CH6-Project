// AO_Spawner_AIManager.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "AI/Base/AO_AICharacterBase.h"
#include "AI/Area/AO_Area_SpawnRestriction.h"
#include "AI/Area/AO_Area_SpawnIntensive.h"
#include "AO_Spawner_AIManager.generated.h"

class UEnvQuery;

/**
 * AI 스포너 매니저
 * - EQS를 사용하여 플레이어 시야에서 보이지 않는 위치에 AI 스폰
 * - 레벨별 몬스터 종류 선택 가능 (1~7종류)
 * - 최대 스폰 수 제한
 * - 특정 영역 스폰 금지/집중 스폰 지원
 */
UCLASS()
class AO_API AAO_Spawner_AIManager : public AActor
{
	GENERATED_BODY()

public:
	AAO_Spawner_AIManager();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 스폰 시도
	void TrySpawnAI();

	// EQS 쿼리 완료 콜백
	void OnEQSQueryFinished(TSharedPtr<FEnvQueryResult> Result);

	// 실제 스폰 실행
	void ExecuteSpawn(const FVector& SpawnLocation, TSubclassOf<AAO_AICharacterBase> MonsterClass);

	// 집중 스폰 영역 체크 및 처리
	void CheckIntensiveSpawnAreas();

	// 집중 스폰 영역 스폰 시도 (볼륨 기반)
	void TrySpawnInIntensiveArea(AAO_Area_SpawnIntensive* IntensiveArea);

	// NavArea 기반 집중 스폰 영역 체크 및 처리
	void CheckIntensiveNavAreas();

	// 현재 스폰된 AI 수 계산
	int32 GetCurrentSpawnedAICount() const;

	// 플레이어 목록 업데이트
	void UpdatePlayerList();

	// AI가 제거되었을 때 호출 (델리게이트 바인딩)
	UFUNCTION()
	void OnAIDestroyed(AActor* DestroyedActor);

public:
	// 스폰 설정 - 몬스터 종류 (1~7개)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn|Monsters", meta = (ArraySizeEnum = "7"))
	TArray<TSubclassOf<AAO_AICharacterBase>> MonsterClasses;

	// 스폰 설정 - 최대 몬스터 수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn|Limits")
	int32 MaxMonstersInLevel = 10;

	// 스폰 설정 - 기본 스폰 간격 (초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn|Timing")
	float SpawnInterval = 5.0f;

	// 스폰 설정 - 랜덤 편차 (초, ±값)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn|Timing", meta = (ClampMin = "0.0"))
	float SpawnIntervalRandomDeviation = 2.0f;

	// EQS 쿼리 에셋
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn|EQS")
	TObjectPtr<UEnvQuery> SpawnLocationQuery;

	// 스폰 금지 영역 목록
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn|Areas")
	TArray<TObjectPtr<AAO_Area_SpawnRestriction>> RestrictedVolumes;

	// 집중 스폰 영역 목록 (볼륨 기반)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn|Areas")
	TArray<TObjectPtr<AAO_Area_SpawnIntensive>> IntensiveVolumes;

	// NavArea 기반 집중 스폰 활성화 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn|Areas|NavArea")
	bool bUseNavAreaIntensiveSpawn = false;

	// NavArea 기반 집중 스폰 간격 (초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn|Areas|NavArea", meta = (ClampMin = "0.1", EditCondition = "bUseNavAreaIntensiveSpawn"))
	float NavAreaIntensiveSpawnInterval = 2.0f;

	// NavArea 기반 집중 스폰 간격 랜덤 편차 (초, ±값)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn|Areas|NavArea", meta = (ClampMin = "0.0", EditCondition = "bUseNavAreaIntensiveSpawn"))
	float NavAreaIntensiveSpawnIntervalRandomDeviation = 1.0f;

	// 디버그: 즉시 스폰
	UFUNCTION(CallInEditor, Category = "Spawn|Debug")
	void DebugSpawn();

protected:
	// 현재 스폰된 AI 목록
	UPROPERTY()
	TArray<TObjectPtr<AAO_AICharacterBase>> SpawnedMonsters;

	// 스폰 타이머 핸들
	UPROPERTY()
	FTimerHandle SpawnTimerHandle;

	// 집중 스폰 영역별 타이머 핸들
	UPROPERTY()
	TMap<TObjectPtr<AAO_Area_SpawnIntensive>, FTimerHandle> IntensiveSpawnTimers;

	// 현재 플레이어 목록 (캐싱)
	UPROPERTY()
	TArray<TObjectPtr<class AAO_PlayerCharacter>> CachedPlayers;
};

