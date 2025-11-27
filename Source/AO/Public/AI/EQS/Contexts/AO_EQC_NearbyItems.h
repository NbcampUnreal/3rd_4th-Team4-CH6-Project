// KSJ : AO_EQC_NearbyItems.h

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "AO_EQC_NearbyItems.generated.h"

/**
 * EQS 컨텍스트: 주변에 있는 '주울 수 있는 아이템' 목록 반환
 * - AO_PickupComponent가 있고
 * - 현재 누군가에게 들려있지 않으며 (IsPickedUp == false)
 * - Ignored 태그가 없는 아이템만 반환
 */
UCLASS()
class AO_API UAO_EQC_NearbyItems : public UEnvQueryContext
{
	GENERATED_BODY()

public:
	UAO_EQC_NearbyItems();

	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;

protected:
	// 탐색 반경
	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	float SearchRadius = 3000.0f;
};

