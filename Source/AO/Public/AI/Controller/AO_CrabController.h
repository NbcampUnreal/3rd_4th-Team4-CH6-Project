// AO_CrabController.h

#pragma once

#include "CoreMinimal.h"
#include "AI/Controller/AO_AIControllerBase.h"
#include "AO_CrabController.generated.h"

class AAO_Crab;
class AAO_MasterItem;

/**
 * Crab AI Controller
 * 
 * 역할:
 * - 플레이어 감지 시 도망 모드 활성화
 * - 소리 감지 시 해당 위치 피하기
 * - State Tree와 연동하여 행동 제어
 */
UCLASS()
class AO_API AAO_CrabController : public AAO_AIControllerBase
{
	GENERATED_BODY()

public:
	AAO_CrabController();

	// 소유한 Crab 가져오기
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Crab")
	AAO_Crab* GetCrab() const;

	// 주변에 플레이어가 있는지 (도망 필요 여부)
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Crab")
	bool ShouldFlee() const;

	// 가장 가까운 위협(플레이어) 위치
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Crab")
	FVector GetNearestThreatLocation() const;

	// 도망 위치 계산 (플레이어로부터 먼 곳)
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Crab")
	FVector CalculateFleeLocation() const;

	// 아이템 탐색 반경
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AO|AI|Crab")
	float ItemSearchRadius = 1500.f;

	// 발견한 아이템 목록 (Crab 전용)
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Crab|Item")
	TArray<AAO_MasterItem*> GetDiscoveredItems() const;

	// 시야 내 아이템 존재 여부
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Crab|Item")
	bool HasItemInSight() const;

	// 가장 가까운 발견된 아이템
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Crab|Item")
	AAO_MasterItem* GetNearestDiscoveredItem() const;

protected:
	virtual void OnPossess(APawn* InPawn) override;

	// 부모 클래스 이벤트 오버라이드
	virtual void OnPlayerDetected(AAO_PlayerCharacter* Player, const FVector& Location) override;
	virtual void OnPlayerLost(AAO_PlayerCharacter* Player, const FVector& LastKnownLocation) override;
	virtual void OnNoiseHeard(AActor* NoiseInstigator, const FVector& Location, float Volume) override;
	virtual void OnActorDetected(AActor* Actor, const FVector& Location) override;

	// 아이템 감지 이벤트 (Crab 전용)
	void OnItemDetected(AAO_MasterItem* Item, const FVector& Location);
	void OnItemLost(AAO_MasterItem* Item);

	// 도망 상태 업데이트
	void UpdateFleeState();

protected:
	// 소유한 Crab 캐시
	UPROPERTY()
	TWeakObjectPtr<AAO_Crab> CachedCrab;

	// 도망 탐색 반경
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AO|AI|Crab|Flee")
	float FleeSearchRadius = 1500.f;

	// 플레이어 근접 감지 거리 (이 거리 안에 오면 도망)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AO|AI|Crab|Flee")
	float PlayerProximityThreshold = 500.f;

	// 발견한 아이템들 (시야에서 사라져도 기억)
	UPROPERTY()
	TArray<TWeakObjectPtr<AAO_MasterItem>> DiscoveredItems;
};
