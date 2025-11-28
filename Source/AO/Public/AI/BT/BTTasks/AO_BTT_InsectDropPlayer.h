// KSJ : AO_BTT_InsectDropPlayer.h

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "AO_BTT_InsectDropPlayer.generated.h"

/**
 * Insect 플레이어 떨어뜨리기 Task
 * - 납치 종료
 * - 넉백 적용
 * - MovementComponent 복구
 * - 쿨다운 등록
 */
UCLASS()
class AO_API UAO_BTT_InsectDropPlayer : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UAO_BTT_InsectDropPlayer();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Kidnap")
	bool bApplyKnockback = true;
};
