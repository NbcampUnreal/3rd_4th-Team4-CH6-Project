// AO_StalkerController.h

#pragma once

#include "CoreMinimal.h"
#include "AI/Controller/AO_AggressiveAICtrl.h"
#include "AO_StalkerController.generated.h"

class AAO_Stalker;

/**
 * Stalker AI Controller
 * - 은신 점수 기반 엄폐물 탐색
 * - 플레이어 시야 고려한 접근
 */
UCLASS()
class AO_API AAO_StalkerController : public AAO_AggressiveAICtrl
{
	GENERATED_BODY()
	
public:
	AAO_StalkerController();

	UFUNCTION(BlueprintCallable, Category = "AO|AI|Stalker")
	AAO_Stalker* GetStalker() const;

	// 엄폐 위치 찾기 (Hide)
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Stalker")
	FVector FindHideLocation(float Radius, AActor* TargetToHideFrom);

	// 공격 후 도망갈 위치 찾기
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Stalker")
	FVector FindRetreatLocation();

protected:
	virtual void OnPossess(APawn* InPawn) override;
};

