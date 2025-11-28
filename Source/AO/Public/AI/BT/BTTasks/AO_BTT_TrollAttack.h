// KSJ : AO_BTT_TrollAttack.h

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "AO_BTT_TrollAttack.generated.h"

/**
 * Troll 공격 실행 태스크
 * - PerformAttack 호출
 */
UCLASS()
class AO_API UAO_BTT_TrollAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UAO_BTT_TrollAttack();
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};