//KSJ : AO_EQS_Test_PlayerFOV

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "AO_EQS_Test_PlayerFOV.generated.h"

/**
 * EQS Test: 모든 플레이어의 시야각(FOV) 밖에 있는지 체크
 * 플레이어가 바라보는 방향의 시야각 내부면 실패
 * 리썰 컴퍼니/REPO 스타일 스폰에 사용
 */
UCLASS()
class AO_API UAO_EQS_Test_PlayerFOV : public UEnvQueryTest
{
	GENERATED_BODY()

public:
	UAO_EQS_Test_PlayerFOV();

	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;

	// 플레이어 시야각 (기본 90도, 절반 각도로 계산됨)
	UPROPERTY(EditDefaultsOnly, Category = "FOV")
	float PlayerFOVDegrees = 90.0f;

	// 최소 거리 (이 거리 이내면 FOV 무관하게 실패 - 바로 옆/뒤 스폰 방지)
	UPROPERTY(EditDefaultsOnly, Category = "FOV")
	float MinDistanceFromPlayer = 500.0f;

	// 카메라 방향 사용 여부 (false면 캐릭터 Forward 사용)
	UPROPERTY(EditDefaultsOnly, Category = "FOV")
	bool bUseCameraDirection = true;
};

