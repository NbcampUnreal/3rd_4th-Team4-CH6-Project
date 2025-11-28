// KSJ : AO_Manager_InsectKidnapManager.h

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "AO_Manager_InsectKidnapManager.generated.h"

class AActor;

/**
 * Insect 납치 시스템을 전역으로 관리하는 매니저 (WorldSubsystem)
 * 
 * [역할]
 * 1. 납치 쿨다운 관리 (어떤 Insect든 같은 플레이어를 일정 시간 동안 납치 불가)
 * 2. 현재 납치 중인 플레이어 추적 (다른 Insect가 납치 시도 방지)
 */
UCLASS()
class AO_API UAO_Manager_InsectKidnapManager : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// --- Kidnap Cooldown ---
	
	/** 플레이어가 납치 가능한지 확인 (쿨다운 체크) */
	bool CanKidnapPlayer(AActor* PlayerActor) const;
	
	/** 플레이어 납치 쿨다운 등록 (납치 종료 시 호출) */
	void RegisterKidnapCooldown(AActor* PlayerActor);
	
	/** 플레이어의 납치 쿨다운 해제 (테스트용) */
	void ClearKidnapCooldown(AActor* PlayerActor);
	
	// --- Active Kidnap Tracking ---
	
	/** 플레이어가 현재 납치 중인지 확인 */
	bool IsPlayerKidnapped(AActor* PlayerActor) const;
	
	/** 납치 시작 등록 */
	void RegisterKidnap(AActor* PlayerActor);
	
	/** 납치 종료 등록 */
	void UnregisterKidnap(AActor* PlayerActor);

private:
	/** 납치 쿨다운 관리 (플레이어 -> 쿨다운 종료 시간) */
	UPROPERTY()
	TMap<TObjectPtr<AActor>, double> KidnapCooldownMap;
	
	/** 현재 납치 중인 플레이어 목록 */
	UPROPERTY()
	TSet<TObjectPtr<AActor>> KidnappedPlayers;
	
	/** 납치 쿨다운 시간 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "Kidnap")
	float KidnapCooldownDuration = 15.0f;
	
	/** 쿨다운 정리 주기 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "Kidnap")
	float CooldownCleanupInterval = 5.0f;
	
	/** 쿨다운 정리 타이머 */
	FTimerHandle CooldownCleanupTimerHandle;
	
	/** 만료된 쿨다운 정리 */
	void CleanupExpiredCooldowns();
};
