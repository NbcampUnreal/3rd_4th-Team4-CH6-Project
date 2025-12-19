// AO_AggressiveAICtrl.h

#pragma once

#include "CoreMinimal.h"
#include "AI/Controller/AO_AIControllerBase.h"
#include "AO_AggressiveAICtrl.generated.h"

class AAO_AggressiveAIBase;
class AAO_PlayerCharacter;

/**
 * 선공형 몬스터 공통 AI Controller
 * 
 * 역할:
 * - 플레이어 발견 시 추격 모드 전환
 * - 플레이어 시야 이탈 시 수색 모드 전환
 * - 다중 플레이어 중 가장 가까운 대상 선택
 * - 경로 충돌 방지 (RVO Avoidance)
 */
UCLASS()
class AO_API AAO_AggressiveAICtrl : public AAO_AIControllerBase
{
	GENERATED_BODY()

public:
	AAO_AggressiveAICtrl();

	// 소유한 AggressiveAI 가져오기
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Aggressive")
	AAO_AggressiveAIBase* GetAggressiveAI() const;

	// 추격 대상 설정/가져오기
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Aggressive")
	AAO_PlayerCharacter* GetChaseTarget() const;

	UFUNCTION(BlueprintCallable, Category = "AO|AI|Aggressive")
	void SetChaseTarget(AAO_PlayerCharacter* NewTarget);

	// 플레이어 발견 시 추격 시작
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Aggressive")
	void StartChase(AAO_PlayerCharacter* Target);

	// 추격 중 대상 교체 (더 가까운 플레이어 발견 시)
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Aggressive")
	void UpdateChaseTargetToNearest();

	// 마지막으로 플레이어를 본 위치
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Aggressive")
	FVector GetLastKnownTargetLocation() const { return LastKnownTargetLocation; }

	// 수색 시작
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Aggressive")
	void StartSearch();

	// 수색 완료 (배회로 전환)
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Aggressive")
	void EndSearch();

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

	// 플레이어 감지 이벤트 오버라이드
	virtual void OnPlayerDetected(AAO_PlayerCharacter* Player, const FVector& Location) override;
	virtual void OnPlayerLost(AAO_PlayerCharacter* Player, const FVector& LastKnownLocation) override;

	// 수색 타이머 완료
	void OnSearchTimerExpired();

protected:
	// 현재 추격 대상
	UPROPERTY()
	TWeakObjectPtr<AAO_PlayerCharacter> ChaseTarget;

	// 마지막으로 플레이어를 본 위치
	UPROPERTY(BlueprintReadOnly, Category = "AO|AI|Aggressive")
	FVector LastKnownTargetLocation = FVector::ZeroVector;

	// 수색 타이머 핸들
	FTimerHandle SearchTimerHandle;

	// 대상 고정 여부 (Stalker처럼 처음 대상을 유지하는 경우)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AO|AI|Aggressive")
	bool bLockOnFirstTarget = false;
};
