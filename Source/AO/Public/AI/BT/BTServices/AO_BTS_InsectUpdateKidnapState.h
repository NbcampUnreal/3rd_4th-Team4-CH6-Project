// KSJ : AO_BTS_InsectUpdateKidnapState.h

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "AO_BTS_InsectUpdateKidnapState.generated.h"

/**
 * Insect 납치 상태 업데이트 서비스
 * - 납치 상태 블랙보드 업데이트
 * - 납치 중 다른 플레이어 감지 시 목표 위치 갱신
 */
UCLASS()
class AO_API UAO_BTS_InsectUpdateKidnapState : public UBTService
{
	GENERATED_BODY()

public:
	UAO_BTS_InsectUpdateKidnapState();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector bIsKidnappingKey;
	
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector KidnappedPlayerKey;
	
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector KidnapStartLocationKey;
	
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector KidnapTargetLocationKey;
	
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector TargetPlayerKey;

private:
	/** 다른 플레이어 감지 범위 (cm) */
	UPROPERTY(EditAnywhere, Category = "Kidnap")
	float OtherPlayerDetectionRadius = 600.0f;
	
	/** 목표 위치 갱신 주기 (초) */
	UPROPERTY(EditAnywhere, Category = "Kidnap")
	float TargetLocationUpdateInterval = 1.0f;
	
	double LastTargetUpdateTime = 0.0;
};
