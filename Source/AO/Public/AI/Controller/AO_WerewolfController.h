//KSJ : AO_WerewolfController

#pragma once

#include "CoreMinimal.h"
#include "AI/Controller/AO_AggressiveAICtrl.h"
#include "AO_WerewolfController.generated.h"

class UAO_PackCoordComp;

/**
 * Werewolf Controller
 * - 최초 발견 시 Howl 수행
 * - 포위/도주로 차단 EQS 활용
 */
UCLASS()
class AO_API AAO_WerewolfController : public AAO_AggressiveAICtrl
{
	GENERATED_BODY()
	
public:
	AAO_WerewolfController();

	// Howl 강제 실행 (StateTree와 독립적으로 실행)
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Werewolf")
	void TriggerHowl(AAO_PlayerCharacter* Target);

	// 도주로 차단 위치 계산
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Werewolf")
	FVector FindEscapeRouteBlockPosition(AAO_PlayerCharacter* Target, float BlockRadius = 500.f);

	// 플레이어 이동 방향 분석
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Werewolf")
	FVector AnalyzePlayerMovementDirection(AAO_PlayerCharacter* Target);

	// 잠재적 도주 경로 찾기
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Werewolf")
	TArray<FVector> FindPotentialEscapeRoutes(AAO_PlayerCharacter* Target, float SearchRadius = 1000.f);

	// 개선된 잠재적 도주 경로 찾기 (8방향 이상 샘플링)
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Werewolf")
	TArray<FVector> FindPotentialEscapeRoutesImproved(AAO_PlayerCharacter* Target, float SearchRadius = 1000.f, int32 NumSamples = 8);

	// Howl을 실행했거나 참여했는지 여부 (StateTree Evaluator에서 접근용)
	UFUNCTION(BlueprintPure, Category = "AO|AI|Werewolf")
	bool HasHowledOrJoined() const { return bHasHowledOrJoined; }

	// Howl/참여 상태 마킹 (Howl Task에서 호출)
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Werewolf")
	void MarkHowledOrJoined() { bHasHowledOrJoined = true; }

	// Howl/참여 상태 리셋 (배회로 돌아갈 때 사용)
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Werewolf")
	void ResetHowlState();

protected:
	virtual void OnPossess(APawn* InPawn) override;

	// 플레이어 감지 오버라이드 (Howl 로직)
	virtual void OnPlayerDetected(AAO_PlayerCharacter* Player, const FVector& Location) override;
	
	// 플레이어 놓침 오버라이드 (포위 모드 해제)
	virtual void OnPlayerLost(AAO_PlayerCharacter* Player, const FVector& LastKnownLocation) override;

	// 수색 완료 오버라이드 (Howl 상태 리셋)
	virtual void EndSearch() override;

	// Howl 수신 시 처리
	UFUNCTION()
	void HandleHowlReceived(AActor* TargetActor);

	// 일제공격 시작 이벤트 핸들러
	UFUNCTION()
	void HandleCoordinatedAttackStarted();

protected:
	UPROPERTY()
	TObjectPtr<UAO_PackCoordComp> PackComp;

	// 이미 Howl을 했거나 참여했는지 여부 (상태 관리용)
	bool bHasHowledOrJoined = false;

	// 플레이어 이동 방향 추적용 (최근 위치)
	FVector LastPlayerLocation = FVector::ZeroVector;
	float LastLocationUpdateTime = 0.f;
};
