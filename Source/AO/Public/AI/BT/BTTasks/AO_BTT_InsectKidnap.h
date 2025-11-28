// KSJ : AO_BTT_InsectKidnap.h

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "AO_BTT_InsectKidnap.generated.h"

/**
 * Insect 납치 시작 Task
 * - 근접 거리 진입 시 플레이어 납치
 * - 플레이어를 PickupSocket에 부착
 * - MovementComponent MaxWalkSpeed = 0 설정
 */
UCLASS()
class AO_API UAO_BTT_InsectKidnap : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UAO_BTT_InsectKidnap();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector TargetPlayerKey;
};
