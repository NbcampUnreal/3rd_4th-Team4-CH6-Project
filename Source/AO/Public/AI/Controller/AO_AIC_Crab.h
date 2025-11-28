// KSJ : AO_AIC_Crab.h

#pragma once

#include "CoreMinimal.h"
#include "AI/Controller/AO_AIC_EnemyBase.h" // Base 클래스 포함
#include "AO_AIC_Crab.generated.h"

/**
 * Crab 전용 AI Controller
 * - AO_AIC_EnemyBase 상속
 * - 시각 감지 및 블랙보드 업데이트 (Base 클래스 활용)
 */
UCLASS()
class AO_API AAO_AIC_Crab : public AAO_AIC_EnemyBase
{
	GENERATED_BODY()

public:
	AAO_AIC_Crab();

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void HandleSight(AActor* Actor, const FAIStimulus& Stimulus) override;
};