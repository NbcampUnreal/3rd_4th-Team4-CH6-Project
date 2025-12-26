// AO_EQS_Test_LineOfSight.h

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "AO_EQS_Test_LineOfSight.generated.h"

/**
 * EQS Test: 모든 플레이어의 시야에서 보이지 않는 위치인지 체크
 * 모든 플레이어로부터 Line Trace가 막혀야 통과
 */
UCLASS()
class AO_API UAO_EQS_Test_LineOfSight : public UEnvQueryTest
{
	GENERATED_BODY()

public:
	UAO_EQS_Test_LineOfSight();

	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;

	// 플레이어 눈 높이 오프셋 (캐릭터 위치 기준)
	UPROPERTY(EditDefaultsOnly, Category = "LineOfSight")
	float PlayerEyeHeightOffset = 160.0f;

	// 스폰 위치 높이 오프셋 (지면 기준)
	UPROPERTY(EditDefaultsOnly, Category = "LineOfSight")
	float SpawnLocationHeightOffset = 100.0f;

	// Line Trace 채널
	UPROPERTY(EditDefaultsOnly, Category = "LineOfSight")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;
};

