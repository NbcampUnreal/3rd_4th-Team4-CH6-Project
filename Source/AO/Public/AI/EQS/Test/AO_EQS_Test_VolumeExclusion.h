// AO_EQS_Test_VolumeExclusion.h

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "AI/Area/AO_Area_SpawnRestriction.h"
#include "AO_EQS_Test_VolumeExclusion.generated.h"

/**
 * EQS Test: 스폰 금지 영역과의 겹침 체크
 * 스폰 금지 볼륨 내부에 있으면 실패
 */
UCLASS()
class AO_API UAO_EQS_Test_VolumeExclusion : public UEnvQueryTest
{
	GENERATED_BODY()

public:
	UAO_EQS_Test_VolumeExclusion();

	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;

	// 테스트할 볼륨 클래스
	UPROPERTY(EditDefaultsOnly, Category = "VolumeExclusion")
	TSubclassOf<AAO_Area_SpawnRestriction> RestrictionVolumeClass;
};

