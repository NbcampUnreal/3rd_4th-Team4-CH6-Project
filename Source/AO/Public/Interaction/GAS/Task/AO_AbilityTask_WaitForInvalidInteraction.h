// HSJ : AO_AbilityTask_WaitForInvalidInteraction.h
#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AO_AbilityTask_WaitForInvalidInteraction.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInvalidInteraction);

/**
 * 홀딩 상호작용 중 플레이어의 이탈을 감지하는 태스크
 * 
 * 1. 홀딩 시작 시점의 방향과 위치 저장
 * 2. 0.1초마다 현재 상태 체크
 * 3. 허용 범위 초과 시 OnInvalidInteraction 델리게이트 브로드캐스트
 * 
 * - 시선 방향: AcceptanceAngle 이내 유지
 * - XY 이동: AcceptanceDistance 이내 유지
 * - Z 이동: AcceptanceDistance + 앉기 높이 이내 유지 (앉기 허용)
 * 
 * 예시: 상호작용 홀딩 중 뒤돌아보거나 멀리 걸어가면 취소
 */
UCLASS()
class AO_API UAO_AbilityTask_WaitForInvalidInteraction : public UAbilityTask
{
	GENERATED_BODY()

public:
	UAO_AbilityTask_WaitForInvalidInteraction(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// Task 생성
	UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta=(HidePin="OwningAbility", DefaultToSelf="OwningAbility", BlueprintInternalUseOnly="true"))
	static UAO_AbilityTask_WaitForInvalidInteraction* WaitForInvalidInteraction(
		UGameplayAbility* OwningAbility, 
		float AcceptanceAngle, 
		float AcceptanceDistance);

	UPROPERTY(BlueprintAssignable)
	FOnInvalidInteraction OnInvalidInteraction;

protected:
	virtual void Activate() override;
	virtual void OnDestroy(bool bInOwnerFinished) override;

private:
	// 주기적 유효성 검사
	void PerformCheck();

	// 시작 방향 대비 현재 방향의 각도 차이 계산 (2D)
	float CalculateAngle2D() const;

	float AcceptanceAngle = 0.f;
	float AcceptanceDistance = 0.f;
	
	FTimerHandle CheckTimerHandle;
	
	FVector CachedCharacterForward2D;  // 홀딩 시작 시 캐릭터 방향 (2D)
	FVector CachedCharacterLocation;   // 홀딩 시작 시 캐릭터 위치
};