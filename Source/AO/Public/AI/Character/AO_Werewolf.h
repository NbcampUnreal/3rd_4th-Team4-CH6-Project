//KSJ : AO_Werewolf

#pragma once

#include "CoreMinimal.h"
#include "AI/Base/AO_AggressiveAIBase.h"
#include "AO_Werewolf.generated.h"

class UAO_PackCoordComp;

/**
 * Werewolf (늑대인간)
 * - 빠른 이동 속도 (400 / 700)
 * - 무리 행동 (Pack Coordination)
 */
UCLASS()
class AO_API AAO_Werewolf : public AAO_AggressiveAIBase
{
	GENERATED_BODY()
	
public:
	AAO_Werewolf();

	virtual void PostInitializeComponents() override;

	// 무리 행동 컴포넌트 접근자
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Werewolf")
	UAO_PackCoordComp* GetPackCoordComp() const { return PackCoordComp; }

	// 공통 공격 인터페이스 오버라이드
	virtual FEnemyAttackConfig GetCurrentAttackConfig_Implementation() const override;

protected:
	// 기절 처리 오버라이드
	virtual void HandleStunBegin() override;
	virtual void HandleStunEnd() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|AI|Werewolf")
	TObjectPtr<UAO_PackCoordComp> PackCoordComp;

	// 공격 설정 (에디터에서 몽타주 지정)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|AI|Werewolf|Combat")
	FEnemyAttackConfig AttackConfig;
};
