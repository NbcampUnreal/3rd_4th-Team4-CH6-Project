// KSJ : AO_BTS_CheckCrabState.h

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "AO_BTS_CheckCrabState.generated.h"

/**
 * Crab의 상태(짐 들고 있는지)를 주기적으로 체크하고,
 * 들고 있지 않다면 주변 아이템을 탐색해 TargetItem을 갱신하는 서비스
 */
UCLASS()
class AO_API UAO_BTS_CheckCrabState : public UBTService
{
	GENERATED_BODY()

public:
	UAO_BTS_CheckCrabState();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	// 아이템 탐색 반경
	UPROPERTY(EditAnywhere, Category = "AI")
	float SearchRadius = 2000.0f;

	// 블랙보드 키 필터 (선택 사항)
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector TargetItemKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector HeldItemKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector IsBurdenedKey;

private:
	// 경로 재설정 쿨다운
	double LastRerouteTime = 0.0;
};