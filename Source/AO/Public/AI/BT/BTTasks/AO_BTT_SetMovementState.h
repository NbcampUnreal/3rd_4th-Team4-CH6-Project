// AO_BTT_SetMovementSpeed.h

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "AI/Character/AO_Enemy_HumanoidBase.h"
#include "AO_BTT_SetMovementState.generated.h"

UCLASS()
class AO_API UAO_BTT_SetMovementState : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UAO_BTT_SetMovementState();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp,
											uint8* NodeMemory) override;

public:
	UPROPERTY(EditAnywhere, Category = "Movement")
	EEnemyMovementState NewMovementState = EEnemyMovementState::Idle;
};