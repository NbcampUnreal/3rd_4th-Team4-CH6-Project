// HSJ : AO_AbilityTask_WaitInteractionInputRelease.h
#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AO_AbilityTask_WaitInteractionInputRelease.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractionInputReleased);

/**
 * 홀딩 상호작용 중 입력 해제를 감지하는 태스크
 * 
 * 1. InteractionComponent의 OnInteractInputReleased 델리게이트 구독
 * 2. 추가로 0.1초마다 bIsHoldingInteract 폴링 (안전장치)
 * 3. 입력 해제 감지 시 OnReleased 델리게이트 브로드캐스트
 * 
 * - 홀딩 도중 F키 해제 시 어빌리티 취소
 * - 델리게이트와 폴링을 병행하여 누락 방지
 * 
 * 예시: 상호작용 4초 홀딩 중 2초만에 F키 떼면 취소
 */
UCLASS()
class AO_API UAO_AbilityTask_WaitInteractionInputRelease : public UAbilityTask
{
	GENERATED_BODY()

public:
	// Task 생성
	UFUNCTION(BlueprintCallable, Category="Ability|Tasks", 
		meta=(HidePin="OwningAbility", DefaultToSelf="OwningAbility", BlueprintInternalUseOnly="true"))
	static UAO_AbilityTask_WaitInteractionInputRelease* WaitInteractionInputRelease(UGameplayAbility* OwningAbility);

	UPROPERTY(BlueprintAssignable)
	FOnInteractionInputReleased OnReleased;

protected:
	virtual void Activate() override;
	virtual void OnDestroy(bool bInOwnerFinished) override;

private:
	// 입력 해제 콜백
	UFUNCTION()
	void OnInputReleased();
	
	FTimerHandle CheckTimerHandle; // 폴링용 타이머
};