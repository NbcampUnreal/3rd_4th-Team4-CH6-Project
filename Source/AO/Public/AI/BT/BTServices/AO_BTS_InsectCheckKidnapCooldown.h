// KSJ : AO_BTS_InsectCheckKidnapCooldown.h

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "AO_BTS_InsectCheckKidnapCooldown.generated.h"

/**
 * Insect 납치 쿨다운 체크 서비스
 * - Manager에서 플레이어 납치 가능 여부 확인
 * - 블랙보드에 결과 업데이트
 */
UCLASS()
class AO_API UAO_BTS_InsectCheckKidnapCooldown : public UBTService
{
	GENERATED_BODY()

public:
	UAO_BTS_InsectCheckKidnapCooldown();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector TargetPlayerKey;
	
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector bCanKidnapKey;
};
