// AO_STTask_Troll_PickWeapon.h

#pragma once

#include "CoreMinimal.h"
#include "Tasks/StateTreeAITask.h"
#include "AO_STTask_Troll_PickWeapon.generated.h"

class AAO_TrollController;
class AAO_Troll;
class AAO_TrollWeapon;
class UAnimMontage;

/**
 * Troll 무기 줍기 Task 인스턴스 데이터
 */
USTRUCT()
struct FAO_STTask_Troll_PickWeapon_InstanceData
{
	GENERATED_BODY()

	// 도착 허용 반경 (네비게이션 이동 목표)
	UPROPERTY(EditAnywhere, Category = "PickWeapon")
	float AcceptanceRadius = 50.f;

	// 무기 줍기 반경 (이 거리 이내면 줍기 시도)
	UPROPERTY(EditAnywhere, Category = "PickWeapon")
	float PickupRadius = 300.f;

	// 무기 탐색 반경
	UPROPERTY(EditAnywhere, Category = "PickWeapon")
	float SearchRadius = 5000.f;

	// 무기 줍기 몽타주 (설정되어 있으면 재생 후 줍기)
	UPROPERTY(EditAnywhere, Category = "PickWeapon")
	TObjectPtr<UAnimMontage> PickupMontage = nullptr;

	// 현재 이동 중인지
	UPROPERTY()
	bool bIsMoving = false;

	// 무기 줍기 애니메이션 재생 중인지
	UPROPERTY()
	bool bIsPlayingPickupAnimation = false;

	// 목표 무기
	UPROPERTY()
	TWeakObjectPtr<AAO_TrollWeapon> TargetWeapon;
};

/**
 * Troll 무기 줍기 Task
 * - 시야 내 무기로 이동
 * - 무기에 도달하면 줍기
 */
USTRUCT(meta = (DisplayName = "AO Troll Pick Weapon", Category = "AI|AO|Troll"))
struct AO_API FAO_STTask_Troll_PickWeapon : public FStateTreeAIActionTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAO_STTask_Troll_PickWeapon_InstanceData;

	FAO_STTask_Troll_PickWeapon() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

protected:
	AAO_TrollController* GetTrollController(FStateTreeExecutionContext& Context) const;
	AAO_Troll* GetTroll(FStateTreeExecutionContext& Context) const;
	bool MoveToWeapon(AAO_TrollController* Controller, AAO_TrollWeapon* Weapon, float AcceptRadius) const;
};

