// KSJ : AO_BTT_WerewolfAttack.h

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "AO_BTT_WerewolfAttack.generated.h"

/**
 * 웨어울프 공격 Task
 * - 공격 몽타주 재생
 * - 재생 완료 대기
 */
UCLASS()
class AO_API UAO_BTT_WerewolfAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UAO_BTT_WerewolfAttack();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted, UBehaviorTreeComponent* OwnerComp);
};
