// AO_EQS_Test_NavAreaExclusion.h

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "NavAreas/NavArea.h"
#include "AO_EQS_Test_NavAreaExclusion.generated.h"

/**
 * EQS Test: 특정 NavArea와의 겹침 체크
 * 스폰 금지 NavArea 내부에 있으면 실패
 */
UCLASS()
class AO_API UAO_EQS_Test_NavAreaExclusion : public UEnvQueryTest
{
	GENERATED_BODY()

public:
	UAO_EQS_Test_NavAreaExclusion();

	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;

	// 제외할 NavArea 클래스
	UPROPERTY(EditDefaultsOnly, Category = "NavAreaExclusion")
	TSubclassOf<UNavArea> RestrictedNavAreaClass;
};

