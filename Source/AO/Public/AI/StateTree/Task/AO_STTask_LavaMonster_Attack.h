// AO_STTask_LavaMonster_Attack.h

#pragma once

#include "CoreMinimal.h"
#include "Tasks/StateTreeAITask.h"
#include "AO_STTask_LavaMonster_Attack.generated.h"

class AAO_LavaMonsterCtrl;
class AAO_LavaMonster;

/**
 * 용암 몬스터 공격 Task 인스턴스 데이터
 */
USTRUCT()
struct FAO_STTask_LavaMonster_Attack_InstanceData
{
	GENERATED_BODY()

	// 공격 중인지
	UPROPERTY()
	bool bIsAttacking = false;

	// 공격 완료 대기 중인지
	UPROPERTY()
	bool bWaitingForAttackEnd = false;
};

/**
 * 용암 몬스터 공격 Task
 * - 랜덤 공격 타입 선택 및 실행
 * - 공격 완료까지 대기
 */
USTRUCT(meta = (DisplayName = "AO LavaMonster Attack", Category = "AI|AO|LavaMonster"))
struct AO_API FAO_STTask_LavaMonster_Attack : public FStateTreeAIActionTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAO_STTask_LavaMonster_Attack_InstanceData;

	FAO_STTask_LavaMonster_Attack() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

protected:
	AAO_LavaMonsterCtrl* GetLavaMonsterController(FStateTreeExecutionContext& Context) const;
	AAO_LavaMonster* GetLavaMonster(FStateTreeExecutionContext& Context) const;
};

