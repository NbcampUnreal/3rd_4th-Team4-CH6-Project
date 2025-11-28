// KSJ : AO_BTT_RushAttack.h

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "AO_BTT_RushAttack.generated.h"

/**
 * 웨어울프 일제 공격 (Rush) Task
 * - 기본적인 MoveToTarget + Attack 로직
 * - 여기서는 간단히 "이동 후 공격 몽타주 재생"까지만 구현하거나,
 * - 기존 Attack Task를 재활용할 수도 있음.
 */
UCLASS()
class AO_API UAO_BTT_RushAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UAO_BTT_RushAttack();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
