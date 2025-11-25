// HSJ : AO_GameplayAbility_Inspect_Click.h
#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AO_GameplayAbility_Inspect_Click.generated.h"

/**
 * Inspection 모드에서 마우스 클릭 처리 어빌리티
 * 
 * - 로컬에서 즉시 실행 (LocalPredicted) -> 서버에서 검증 및 실제 실행
 * 
 * 1. 마우스 커서 위치에서 Trace
 * 2. Hit된 Component 찾기
 * 3. InspectionComponent->ServerProcessInspectionClick() 호출
 * 4. 서버에서 OnInspectionMeshClicked() 실행
 * 5. PuzzleChecker에 이벤트 전송
 */

UCLASS()
class AO_API UAO_GameplayAbility_Inspect_Click : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UAO_GameplayAbility_Inspect_Click();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Inspection")
	float ClickTraceRange = 10000.0f;
};