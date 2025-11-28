// KSJ : AO_BTT_TrollConfirmTarget.h

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "AO_BTT_TrollConfirmTarget.generated.h"

/**
 * 고민(Hesitate) 끝에 새로운 타겟을 확정하는 태스크
 * - PotentialTarget -> TargetPlayer로 승격
 * - bIsHesitating 초기화
 */
UCLASS()
class AO_API UAO_BTT_TrollConfirmTarget : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UAO_BTT_TrollConfirmTarget();
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};