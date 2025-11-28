// KSJ : AO_BTT_Howl.h

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "AO_BTT_Howl.generated.h"

/**
 * 웨어울프 Howl Task
 * - 제자리에서 Howl 몽타주 재생
 * - 재생이 끝날 때까지 대기
 */
UCLASS()
class AO_API UAO_BTT_Howl : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UAO_BTT_Howl();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted, UBehaviorTreeComponent* OwnerComp);

	FDelegateHandle MontageEndedDelegateHandle;
};
