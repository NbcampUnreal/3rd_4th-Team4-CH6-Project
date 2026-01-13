//KSJ : AO_StalkerController

#pragma once

#include "CoreMinimal.h"
#include "AI/Controller/AO_AggressiveAICtrl.h"
#include "AO_StalkerController.generated.h"

class AAO_Stalker;
class UEnvQuery;

/**
 * Stalker AI Controller
 * - 은신 점수 기반 엄폐물 탐색
 * - 플레이어 시야 고려한 접근
 */
UCLASS()
class AO_API AAO_StalkerController : public AAO_AggressiveAICtrl
{
	GENERATED_BODY()
	
public:
	AAO_StalkerController();

	UFUNCTION(BlueprintCallable, Category = "AO|AI|Stalker")
	AAO_Stalker* GetStalker() const;

	// 엄폐 위치 찾기 (Hide) - 다중 플레이어 시야 고려
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Stalker")
	FVector FindHideLocation(float Radius, AActor* TargetToHideFrom);

	// KSJ: 다중 플레이어 시야를 고려한 엄폐 위치 찾기
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Stalker")
	FVector FindHideLocationFromMultiple(float Radius, const TArray<AActor*>& PlayersToHideFrom);

	// KSJ: 현재 나를 보고 있는 모든 플레이어 목록 반환
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Stalker")
	TArray<AActor*> GetAllLookingPlayers(float ToleranceDegrees = 45.f) const;

	// KSJ: EQS 기반 엄폐 위치 요청 (비동기). 결과는 Controller에 보관되며 StateTree Task가 소비한다.
	void RequestHideLocationEQS(UEnvQuery* Query);
	bool ConsumePendingHideLocation(FVector& OutLocation);

	// 공격 후 도망갈 위치 찾기
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Stalker")
	FVector FindRetreatLocation();

	// 공격 종료 처리 (Hit & Run 시작)
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Stalker")
	void OnAttackFinished();

	// 도주(치고 빠지기) 중인지 여부
	// KSJ: Retreat 상태는 Actor(AAO_Stalker)가 단일 소스로 소유한다.
	// Controller는 타이머/이동만 관리하고, 상태는 Pawn에서 읽는다.
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Stalker")
	bool IsRetreating() const;

	// 플레이어가 나를 보고 있는지 확인
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Stalker")
	bool IsPlayerLookingAtMe(AActor* TargetActor, float ToleranceDegrees = 45.f) const;

	// KSJ: Hysteresis 적용된 LookingPlayer 업데이트 (안정적인 타겟 유지)
	// 반환값: 타겟이 변경되었으면 true
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Stalker")
	bool UpdateLookingPlayerWithHysteresis(float DeltaTime);

	// KSJ: 현재 Hysteresis가 적용된 LookingPlayer 반환
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Stalker")
	AActor* GetStableLookingPlayer() const { return StableLookingPlayer.Get(); }

	// KSJ: 아무 플레이어가 나를 보고 있는지 여부 (Hysteresis 적용)
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Stalker")
	bool IsAnyPlayerLookingAtMe() const { return StableLookingPlayer.IsValid(); }

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnPlayerDetected(AAO_PlayerCharacter* Player, const FVector& Location) override;
	virtual void OnPlayerLost(AAO_PlayerCharacter* Player, const FVector& LastKnownLocation) override;

	// 도주 타이머
	void OnRetreatTimerExpired();
	void OnTargetPersistenceExpired();

protected:
	// 도주 지속 시간
	UPROPERTY(EditDefaultsOnly, Category = "AO|AI|Stalker")
	float RetreatDuration = 5.f;

	// 타겟을 놓친 뒤에도 '스토킹'을 유지할 시간 (시야에서 사라져도 즉시 Search로 넘어가지 않음)
	// KSJ: Stalker 요구사항 - 엄폐 접근 중에는 타겟이 잠깐 시야에서 사라지는 것이 정상이다.
	UPROPERTY(EditDefaultsOnly, Category = "AO|AI|Stalker")
	float TargetPersistenceSeconds = 8.f;

	// KSJ: LookingPlayer Hysteresis 설정
	// 타겟 최소 유지 시간 (빈번한 타겟 변경 방지)
	UPROPERTY(EditDefaultsOnly, Category = "AO|AI|Stalker|Hysteresis")
	float LookingPlayerMinHoldTime = 1.5f;

	// 새 타겟으로 전환하기 위한 거리 비율 (현재 타겟 대비 이 비율 이하로 가까워야 전환)
	// 예: 0.7 = 새 타겟이 현재 타겟보다 30% 이상 가까워야 전환
	UPROPERTY(EditDefaultsOnly, Category = "AO|AI|Stalker|Hysteresis")
	float LookingPlayerSwitchDistanceRatio = 0.7f;

	FTimerHandle RetreatTimerHandle;
	FTimerHandle TargetPersistenceTimerHandle;

private:
	// KSJ: StateTree Task와 비동기 EQS 결과 전달용 (InstanceData 참조 캡처를 피하기 위해 Controller에 저장)
	UPROPERTY()
	FVector PendingHideLocation = FVector::ZeroVector;

	UPROPERTY()
	bool bHasPendingHideLocation = false;

	UPROPERTY()
	bool bHideQueryInFlight = false;

	UPROPERTY()
	uint32 HideQuerySerial = 0;

	// KSJ: Hysteresis 적용된 안정적인 LookingPlayer
	UPROPERTY()
	TWeakObjectPtr<AActor> StableLookingPlayer = nullptr;

	// 현재 타겟 유지 시간
	float LookingPlayerHoldTimer = 0.f;
};

