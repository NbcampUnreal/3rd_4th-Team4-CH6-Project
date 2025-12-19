// AO_STTask_Insect_Drag.h

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "AO_STTask_Insect_Drag.generated.h"

class AAO_Insect;
class AAO_InsectController;

USTRUCT()
struct FAO_STTask_Insect_Drag_InstanceData
{
	GENERATED_BODY()

	// 목표 안전 지점 (Evaluator에서 주입)
	UPROPERTY(EditAnywhere, Category = "Input")
	FVector SafeLocation = FVector::ZeroVector;

	// 도착 판정 거리
	UPROPERTY(EditAnywhere, Category = "Parameter")
	float AcceptanceRadius = 100.f;
};

/**
 * Insect 납치 이동(Drag) Task
 * - 뒷걸음질로 SafeLocation까지 이동
 * - 이동 중 Victim을 바라봄 (SetFocus)
 */
USTRUCT(meta = (DisplayName = "AO Insect Drag Move", Category = "AI|AO"))
struct AO_API FAO_STTask_Insect_Drag : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAO_STTask_Insect_Drag_InstanceData;

	FAO_STTask_Insect_Drag() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
