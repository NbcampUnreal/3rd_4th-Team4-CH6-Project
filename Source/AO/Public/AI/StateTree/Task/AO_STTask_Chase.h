// AO_STTask_Chase.h

#pragma once

#include "CoreMinimal.h"
#include "Tasks/StateTreeAITask.h"
#include "AO_STTask_Chase.generated.h"

class AAO_AggressiveAICtrl;
class AAO_PlayerCharacter;

/**
 * 선공형 AI 추격 Task 인스턴스 데이터
 */
USTRUCT()
struct FAO_STTask_Chase_InstanceData
{
	GENERATED_BODY()

	// 도착 허용 반경
	UPROPERTY(EditAnywhere, Category = "Chase")
	float AcceptanceRadius = 100.f;

	// 경로 재계산 간격
	UPROPERTY(EditAnywhere, Category = "Chase")
	float PathUpdateInterval = 0.3f;

	// 경로 재계산 타이머
	UPROPERTY()
	float PathUpdateTimer = 0.f;

	// 현재 이동 중인지
	UPROPERTY()
	bool bIsMoving = false;
};

/**
 * 선공형 AI 추격 Task
 * - 현재 타겟 플레이어를 추격
 * - 주기적으로 경로 갱신
 * - 공격 범위에 도달하면 성공
 */
USTRUCT(meta = (DisplayName = "AO AI Chase", Category = "AI|AO"))
struct AO_API FAO_STTask_Chase : public FStateTreeAIActionTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAO_STTask_Chase_InstanceData;

	FAO_STTask_Chase() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

protected:
	AAO_AggressiveAICtrl* GetAggressiveController(FStateTreeExecutionContext& Context) const;
	bool StartChasing(AAO_AggressiveAICtrl* Controller, AAO_PlayerCharacter* Target, float AcceptRadius) const;
};

