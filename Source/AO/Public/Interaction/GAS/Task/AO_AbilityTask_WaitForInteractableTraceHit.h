// HSJ : AO_AbilityTask_WaitForInteractableTraceHit.h
#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Interaction/Interface/AO_InteractionQuery.h"
#include "AO_AbilityTask_WaitForInteractableTraceHit.generated.h"

struct FAO_InteractionInfo;
class IAO_Interface_Interactable;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractableChanged, const TArray<FAO_InteractionInfo>&, InteractableInfos);

/**
 * 플레이어 시야 내 상호작용 오브젝트를 추적하는 태스크
 * 
 * 1. 카메라 방향으로 주기적으로 SphereCast 수행
 * 2. 감지된 오브젝트의 상호작용 정보 수집
 * 3. 변화 감지 시 하이라이트 업데이트 + UI 갱신
 * 4. InteractableChanged 델리게이트 브로드캐스트
 * 
 */
UCLASS()
class AO_API UAO_AbilityTask_WaitForInteractableTraceHit : public UAbilityTask
{
	GENERATED_BODY()

public:
	UAO_AbilityTask_WaitForInteractableTraceHit(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// Task 생성
	UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta=(HidePin="OwningAbility", DefaultToSelf="OwningAbility", BlueprintInternalUseOnly="true"))
	static UAO_AbilityTask_WaitForInteractableTraceHit* WaitForInteractableTraceHit(
		UGameplayAbility* OwningAbility, 
		FAO_InteractionQuery InteractionQuery,
		ECollisionChannel TraceChannel, 
		FGameplayAbilityTargetingLocationInfo StartLocation,
		float InteractionTraceRange = 100.f, 
		float InteractionTraceRate = 0.1f,
		bool bShowDebug = false, 
		float SphereRadius = 30.0f);

	UPROPERTY(BlueprintAssignable)
	FOnInteractableChanged InteractableChanged;

protected:
	virtual void Activate() override;
	virtual void OnDestroy(bool bInOwnerFinished) override;

private:
	// 레이캐스트 수행 및 오브젝트 감지
	void PerformTrace();

	// 카메라 시점을 기반으로 트레이스 종료점 계산
	// 카메라 방향과 플레이어 위치 기준 구체 범위를 고려한 정밀 타겟팅
	void AimWithPlayerController(
		const AActor* InSourceActor, 
		FCollisionQueryParams Params, 
		const FVector& TraceStart, 
		float MaxRange, 
		FVector& OutTraceEnd, 
		bool bIgnorePitch = false) const;

	// 카메라 레이를 플레이어 중심 구체 범위 내로 제한
	// 카메라가 멀리 떨어져도 상호작용 범위를 벗어나지 않도록 보장
	bool ClipCameraRayToAbilityRange(
		FVector CameraLocation, 
		FVector CameraDirection, 
		FVector AbilityCenter, 
		float AbilityRange, 
		FVector& OutClippedPosition) const;

	// SphereCast 수행
	void LineTrace(
		const FVector& Start, 
		const FVector& End, 
		const FCollisionQueryParams& Params, 
		FHitResult& OutHitResult) const;
	
	// 감지된 오브젝트들로부터 상호작용 정보 수집 및 업데이트
	// 변화 감지 시 하이라이트 갱신 및 델리게이트 브로드캐스트
	void UpdateInteractionInfos(
		const FAO_InteractionQuery& InteractQuery, 
		const TArray<TScriptInterface<IAO_Interface_Interactable>>& Interactables);

	// 상호작용 오브젝트의 CustomDepth 하이라이트 on/off
	void HighlightInteractables(
		const TArray<FAO_InteractionInfo>& InteractionInfos, 
		bool bShouldHighlight);

	UPROPERTY()
	FAO_InteractionQuery InteractionQuery;

	UPROPERTY()
	FGameplayAbilityTargetingLocationInfo StartLocation;

	ECollisionChannel TraceChannel = ECC_Visibility;
	float InteractionTraceRange = 150.f;
	float InteractionTraceRate = 0.1f;
	bool bShowDebug = false;
	float TraceSphereRadius = 60.0f;
	
	FTimerHandle TraceTimerHandle;

	// 현재 감지된 정보 (변화 감지용)
	TArray<FAO_InteractionInfo> CurrentInteractionInfos;
};