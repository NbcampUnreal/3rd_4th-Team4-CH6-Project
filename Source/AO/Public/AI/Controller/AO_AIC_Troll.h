// KSJ : AO_AIC_Troll.h

#pragma once

#include "CoreMinimal.h"
#include "AI/Controller/AO_AIC_EnemyBase.h"
#include "AO_AIC_Troll.generated.h"

/**
 * 트롤 전용 AI Controller
 * - 시각 감지
 * - 타겟 변경 시 멍청하게 고민(Hesitate)하는 로직 처리
 */
UCLASS()
class AO_API AAO_AIC_Troll : public AAO_AIC_EnemyBase
{
	GENERATED_BODY()

public:
	AAO_AIC_Troll();

	UFUNCTION(BlueprintCallable, Category = "AI|Troll")
	void ConsiderNewTarget(AActor* NewTarget);

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void HandleSight(AActor* Actor, const FAIStimulus& Stimulus) override;

	UPROPERTY(EditAnywhere, Category = "AI|Troll")
	float TargetChangeCooldown = 5.0f;

private:
	double LastTargetChangeTime = 0.0;
	
	UPROPERTY()
	TObjectPtr<AActor> PotentialTarget;
};