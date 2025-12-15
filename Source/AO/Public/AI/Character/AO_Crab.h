// AO_Crab.h

#pragma once

#include "CoreMinimal.h"
#include "AI/Base/AO_AICharacterBase.h"
#include "AO_Crab.generated.h"

class UAO_ItemCarryComponent;
class AAO_MasterItem;

/**
 * Crab AI 캐릭터
 * 
 * 특징:
 * - 레벨을 배회하며 아이템을 찾아다님
 * - 아이템을 주워서 플레이어들로부터 먼 곳으로 옮김
 * - 플레이어를 발견하면 도망감
 * - 기절 시 들고있던 아이템을 떨어뜨림
 * - 전투 능력 없음 (도주형 AI)
 * 
 * 속도:
 * - 기본: 500
 * - 도망: 600
 * - 아이템 소지 시: 300 (고정)
 */
UCLASS()
class AO_API AAO_Crab : public AAO_AICharacterBase
{
	GENERATED_BODY()

public:
	AAO_Crab();

	// 아이템 운반 컴포넌트 접근
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Crab")
	UAO_ItemCarryComponent* GetItemCarryComponent() const { return ItemCarryComponent; }

	// 도망 모드 설정
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Crab")
	void SetFleeMode(bool bFleeing);

	UFUNCTION(BlueprintCallable, Category = "AO|AI|Crab")
	bool IsInFleeMode() const { return bIsFleeingFromPlayer; }

	// 현재 아이템을 들고 있는지
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Crab")
	bool IsCarryingItem() const;

	// 아이템 드롭 위치 계산 (플레이어들로부터 먼 곳)
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Crab")
	FVector CalculateItemDropLocation() const;

protected:
	virtual void BeginPlay() override;

	// 기절 처리 오버라이드
	virtual void HandleStunBegin() override;
	virtual void HandleStunEnd() override;

	// 이동 속도 업데이트
	void UpdateMovementSpeed();

	// 아이템 픽업/드롭 이벤트 핸들러
	UFUNCTION()
	void OnItemPickedUp(AAO_MasterItem* Item);

	UFUNCTION()
	void OnItemDropped(AAO_MasterItem* Item);

protected:
	// 아이템 운반 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|AI|Crab")
	TObjectPtr<UAO_ItemCarryComponent> ItemCarryComponent;

	// 이동 속도 설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AO|AI|Crab|Movement")
	float NormalSpeed = 500.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AO|AI|Crab|Movement")
	float FleeSpeed = 600.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AO|AI|Crab|Movement")
	float CarryingSpeed = 300.f;

	// 아이템 드롭 위치 탐색 반경
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AO|AI|Crab|Item")
	float ItemDropSearchRadius = 2000.f;

	// 플레이어 감지 거리 (이 거리 안에 플레이어가 오면 도망)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AO|AI|Crab|Flee")
	float FleeDetectionRadius = 500.f;

	// 현재 도망 중인지
	UPROPERTY(BlueprintReadOnly, Category = "AO|AI|Crab")
	bool bIsFleeingFromPlayer = false;

	// 아이템 줏을 때 기록한 플레이어 위치들 (드롭 위치 계산용)
	UPROPERTY()
	TArray<FVector> CachedPlayerLocationsOnPickup;
};
