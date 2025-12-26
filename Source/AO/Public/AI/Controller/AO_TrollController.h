// AO_TrollController.h

#pragma once

#include "CoreMinimal.h"
#include "AI/Controller/AO_AggressiveAICtrl.h"
#include "AO_TrollController.generated.h"

class AAO_Troll;
class AAO_TrollWeapon;

/**
 * Troll AI Controller
 * 
 * 역할:
 * - Troll 전용 행동 제어
 * - 무기 감지 및 줍기 우선순위 관리
 * - 공격 범위 진입 시 공격 트리거
 */
UCLASS()
class AO_API AAO_TrollController : public AAO_AggressiveAICtrl
{
	GENERATED_BODY()

public:
	AAO_TrollController();

	// 소유한 Troll 가져오기
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Troll")
	AAO_Troll* GetTroll() const;

	// 무기 줍기 시도
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Troll")
	bool TryPickupWeapon();

	// 무기 줍기 우선 여부 (무기가 없고 시야에 무기가 있을 때)
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Troll")
	bool ShouldPrioritizeWeaponPickup() const;

	// 공격 가능 여부
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Troll")
	bool CanAttack() const;

	// 가장 가까운 무기 위치
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Troll")
	FVector GetNearestWeaponLocation() const;

	// 시야 내 무기 있는지
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Troll")
	bool HasWeaponInSight() const;

protected:
	virtual void OnPossess(APawn* InPawn) override;

	// 액터 감지 오버라이드 (무기 감지용)
	virtual void OnActorDetected(AActor* Actor, const FVector& Location) override;
	virtual void OnActorLost(AActor* Actor) override;

protected:
	// 무기 탐색 반경 (시야 밖에서도 찾을 수 있는 범위)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AO|AI|Troll")
	float WeaponSearchRadius = 1000.f;
};
