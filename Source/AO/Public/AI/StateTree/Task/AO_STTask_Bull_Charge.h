// AO_STTask_Bull_Charge.h

#pragma once

#include "CoreMinimal.h"
#include "Tasks/StateTreeAITask.h"
#include "AO_STTask_Bull_Charge.generated.h"

class AAO_BullController;
class AAO_Bull;
class AAO_PlayerCharacter;

USTRUCT()
struct FAO_STTask_Bull_Charge_InstanceData
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<AAO_BullController> Controller = nullptr;

	UPROPERTY()
	bool bIsCharging = false;

	UPROPERTY()
	TObjectPtr<AAO_PlayerCharacter> TargetPlayer = nullptr;
};

/**
 * Bull 돌진 Task
 * - GAS 돌진 어빌리티 실행
 * - 돌진 완료(어빌리티 종료) 대기
 */
USTRUCT(meta = (DisplayName = "AO Bull Charge", Category = "AI|AO|Bull"))
struct AO_API FAO_STTask_Bull_Charge : public FStateTreeAIActionTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAO_STTask_Bull_Charge_InstanceData;

	FAO_STTask_Bull_Charge() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

protected:
	AAO_BullController* GetController(FStateTreeExecutionContext& Context) const;
};

