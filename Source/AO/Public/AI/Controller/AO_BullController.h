// AO_BullController.h

#pragma once

#include "CoreMinimal.h"
#include "AI/Controller/AO_AggressiveAICtrl.h"
#include "AO_BullController.generated.h"

class AAO_Bull;

/**
 * Bull AI Controller
 */
UCLASS()
class AO_API AAO_BullController : public AAO_AggressiveAICtrl
{
	GENERATED_BODY()
	
public:
	AAO_BullController();

	UFUNCTION(BlueprintCallable, Category = "AO|AI|Bull")
	AAO_Bull* GetBull() const;

	// 돌진 가능 여부 (거리, 쿨타임 등)
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Bull")
	bool CanChargeAttack() const;

protected:
	virtual void OnPossess(APawn* InPawn) override;

protected:
	// 돌진 최소/최대 거리
	UPROPERTY(EditDefaultsOnly, Category = "AO|AI|Bull")
	float MinChargeDistance = 500.f;

	UPROPERTY(EditDefaultsOnly, Category = "AO|AI|Bull")
	float MaxChargeDistance = 2000.f;
};

