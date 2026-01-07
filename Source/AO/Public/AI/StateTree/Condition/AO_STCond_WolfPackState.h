//KSJ : AO_STCond_WolfPackState

#pragma once

#include "CoreMinimal.h"
#include "Conditions/StateTreeAIConditionBase.h"
#include "AO_STCond_WolfPackState.generated.h"

class UAO_PackCoordComp;
class AAO_WerewolfController;

/**
 * Werewolf Pack 상태 체크 타입
 */
UENUM(BlueprintType)
enum class EAO_WolfPackStateCheck : uint8
{
	// Howl을 실행했거나 수신했는지 (처음 발견 후 Howl 필요 여부 판단)
	HasHowledOrJoined,
	
	// 현재 포위 모드인지
	IsSurrounding,
	
	// 일제공격이 시작되었는지
	IsCoordinatedAttackStarted,
	
	// Howl 실행자인지 (처음 발견한 늑대)
	IsHowlInitiator,
	
	// 포위 위치에 도달했는지
	HasReachedSurroundPosition,
	
	// 동료가 있는지 (PackMemberCount > 0)
	HasPackMembers
};

/**
 * Werewolf Pack 상태 Condition 인스턴스 데이터
 */
USTRUCT()
struct FAO_STCond_WolfPackState_InstanceData
{
	GENERATED_BODY()

	// 체크할 상태 타입
	UPROPERTY(EditAnywhere, Category = "Condition")
	EAO_WolfPackStateCheck StateToCheck = EAO_WolfPackStateCheck::IsSurrounding;

	// 반전 여부
	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bInvert = false;

	// 캐시된 컴포넌트
	UPROPERTY()
	TObjectPtr<UAO_PackCoordComp> PackComp = nullptr;

	UPROPERTY()
	TObjectPtr<AAO_WerewolfController> Controller = nullptr;
};

/**
 * Werewolf Pack 상태 Condition
 * - 다양한 Pack 관련 상태를 체크할 수 있는 범용 Condition
 */
USTRUCT(meta = (DisplayName = "Werewolf: Pack State Check", Category = "AO|Werewolf"))
struct AO_API FAO_STCond_WolfPackState : public FStateTreeAIConditionBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAO_STCond_WolfPackState_InstanceData;

	FAO_STCond_WolfPackState() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
	virtual void EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};

