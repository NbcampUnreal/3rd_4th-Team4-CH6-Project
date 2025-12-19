// AO_Area_SpawnIntensive.h

#pragma once

#include "CoreMinimal.h"
#include "NavModifierVolume.h"
#include "AI/Base/AO_AICharacterBase.h"
#include "AO_Area_SpawnIntensive.generated.h"

/**
 * 집중 스폰 영역
 * 이 볼륨 내부에서는 최대 수 제한을 무시하고 잦은 빈도로 스폰됩니다.
 */
UCLASS()
class AO_API AAO_Area_SpawnIntensive : public ANavModifierVolume
{
	GENERATED_BODY()

public:
	AAO_Area_SpawnIntensive(const FObjectInitializer& ObjectInitializer);

	// 이 영역에서 스폰할 수 있는 몬스터 종류 (비어있으면 스포너의 기본 종류 사용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intensive Spawn", meta = (ArraySizeEnum = "7"))
	TArray<TSubclassOf<AAO_AICharacterBase>> AllowedMonsterClasses;

	// 집중 스폰 활성화 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intensive Spawn")
	bool bEnabled = true;

	// 집중 스폰 간격 (초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intensive Spawn", meta = (ClampMin = "0.1"))
	float IntensiveSpawnInterval = 2.0f;

	// 집중 스폰 간격 랜덤 편차 (초, ±값)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intensive Spawn", meta = (ClampMin = "0.0"))
	float IntensiveSpawnIntervalRandomDeviation = 1.0f;

	// 이 영역 내 최대 스폰 수 (0이면 무제한)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intensive Spawn", meta = (ClampMin = "0"))
	int32 MaxMonstersInArea = 0;

	// 현재 이 영역 내 스폰된 AI 수 반환
	UFUNCTION(BlueprintCallable, Category = "Intensive Spawn")
	int32 GetCurrentMonsterCountInArea() const;
};

