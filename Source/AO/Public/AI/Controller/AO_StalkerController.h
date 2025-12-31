//KSJ : AO_StalkerController

#pragma once

#include "CoreMinimal.h"
#include "AI/Controller/AO_AggressiveAICtrl.h"
#include "AO_StalkerController.generated.h"

class AAO_Stalker;

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

	// 엄폐 위치 찾기 (Hide)
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Stalker")
	FVector FindHideLocation(float Radius, AActor* TargetToHideFrom);

	// 공격 후 도망갈 위치 찾기
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Stalker")
	FVector FindRetreatLocation();

	// 공격 종료 처리 (Hit & Run 시작)
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Stalker")
	void OnAttackFinished();

	// 도주 중인지 여부
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Stalker")
	bool IsRetreating() const { return bIsRetreating; }

	// 플레이어가 나를 보고 있는지 확인
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Stalker")
	bool IsPlayerLookingAtMe(AActor* TargetActor, float ToleranceDegrees = 45.f) const;

protected:
	virtual void OnPossess(APawn* InPawn) override;

	// 도주 타이머
	void OnRetreatTimerExpired();

protected:
	// 도주 상태 플래그
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|AI|Stalker")
	bool bIsRetreating = false;

	// 도주 지속 시간
	UPROPERTY(EditDefaultsOnly, Category = "AO|AI|Stalker")
	float RetreatDuration = 5.f;

	FTimerHandle RetreatTimerHandle;
};

