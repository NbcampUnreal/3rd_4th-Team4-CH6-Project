// HSJ : AO_WorldInteractable.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/Interface/AO_Interface_Interactable.h"
#include "AO_WorldInteractable.generated.h"

/**
 * 월드에 배치 가능한 상호작용 오브젝트의 베이스 클래스
 * 
 * - 홀딩 시작/종료 알림 (OnInteractActiveStarted/Ended)
 * - 상호작용 성공 처리 (OnInteractionSuccess)
 * - 소모성 상호작용 지원 (bShouldConsume)
 * - 블루프린트 이벤트 연동 (K2_ 함수들)
 * 
 * 소모성 동작:
 * - bShouldConsume = true: 한 번만 상호작용 가능
 * - 다른 플레이어가 홀딩 중이면 먼저 성공한 플레이어가 독점
 * - 다른 플레이어들의 상호작용 어빌리티 자동 취소
 * 
 * 사용 예:
 * - 퍼즐 버튼, 레버, 문 등
 */
UCLASS()
class AO_API AAO_WorldInteractable : public AActor, public IAO_Interface_Interactable
{
	GENERATED_BODY()

public:
	AAO_WorldInteractable(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 홀딩 시작 알림
	UFUNCTION(BlueprintCallable)
	virtual void OnInteractActiveStarted(AActor* Interactor);

	UFUNCTION(BlueprintImplementableEvent, DisplayName="OnInteractActiveStarted")
	void K2_OnInteractActiveStarted(AActor* Interactor);

	// 홀딩 종료 알림
	UFUNCTION(BlueprintCallable)
	virtual void OnInteractActiveEnded(AActor* Interactor);

	UFUNCTION(BlueprintImplementableEvent, DisplayName="OnInteractActiveEnded")
	void K2_OnInteractActiveEnded(AActor* Interactor);

	// 상호작용 성공 처리
	UFUNCTION(BlueprintCallable)
	virtual void OnInteractionSuccess(AActor* Interactor);
	
	UFUNCTION(BlueprintImplementableEvent, DisplayName="OnInteractionSuccess")
	void K2_OnInteractionSuccess(AActor* Interactor);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool CanInteraction(const FAO_InteractionQuery& InteractionQuery) const override;

	// 소모 상태 변경 콜백
	UFUNCTION()
	virtual void OnRep_WasConsumed();

	UPROPERTY(EditDefaultsOnly, Category="Interaction")
	bool bShouldConsume = false; // true면 한 번만 상호작용 가능

	UPROPERTY(BlueprintReadWrite, ReplicatedUsing=OnRep_WasConsumed, Category="Interaction")
	bool bWasConsumed = false; // 이미 소모되었는지 여부

	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> CachedInteractors; // 현재 홀딩 중인 플레이어들
};