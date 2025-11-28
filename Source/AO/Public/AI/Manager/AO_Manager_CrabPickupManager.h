// KSJ : AO_Manager_CrabPickupManager.h

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "AO_Manager_CrabPickupManager.generated.h"

class AActor;
class AAO_Enemy_Crab;

/**
 * Crab 아이템 픽업 시스템을 전역으로 관리하는 매니저 (WorldSubsystem)
 * 
 * [역할]
 * 1. 아이템 소유권 관리 (어떤 아이템이 어떤 Crab에 의해 집혔는지 추적)
 * 2. 동시 픽업 충돌 방지 (여러 Crab이 같은 아이템을 집으려 할 때)
 * 3. 아이템 쿨다운 관리 (떨어뜨린 아이템은 일정 시간 재습득 불가)
 */
UCLASS()
class AO_API UAO_Manager_CrabPickupManager : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// --- Item Ownership ---
	
	/** 아이템이 집을 수 있는지 확인 (소유권 및 쿨다운 체크) */
	bool CanPickupItem(AActor* ItemActor) const;
	
	/** 아이템 픽업 시도 (소유권 할당) - 성공 시 true 반환 */
	bool TryClaimItem(AActor* ItemActor, AAO_Enemy_Crab* ClaimingCrab);
	
	/** 아이템 소유권 해제 (아이템을 떨어뜨렸을 때) */
	void ReleaseItem(AActor* ItemActor);
	
	/** 현재 아이템을 집고 있는 Crab 반환 (없으면 nullptr) */
	AAO_Enemy_Crab* GetItemOwner(AActor* ItemActor) const;
	
	/** 특정 Crab이 집고 있는 아이템 반환 (없으면 nullptr) */
	AActor* GetCrabHeldItem(AAO_Enemy_Crab* Crab) const;

private:
	/** 아이템 소유권 관리 (아이템 -> 집고 있는 Crab) */
	UPROPERTY()
	TMap<TObjectPtr<AActor>, TObjectPtr<AAO_Enemy_Crab>> ItemOwnershipMap;
	
	/** 아이템 쿨다운 관리 (아이템 -> 쿨다운 종료 시간) */
	UPROPERTY()
	TMap<TObjectPtr<AActor>, double> ItemCooldownMap;
	
	/** 아이템 쿨다운 시간 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	float ItemCooldownDuration = 5.0f;
	
	/** 쿨다운 정리 주기 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	float CooldownCleanupInterval = 5.0f;
	
	/** 쿨다운 정리 타이머 */
	FTimerHandle CooldownCleanupTimerHandle;
	
	/** 만료된 쿨다운 정리 */
	void CleanupExpiredCooldowns();
};

