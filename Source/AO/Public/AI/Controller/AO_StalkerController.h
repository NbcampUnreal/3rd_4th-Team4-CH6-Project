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

	// 엄폐 위치 찾기 (Hide)
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Stalker")
	FVector FindHideLocation(float Radius, AActor* TargetToHideFrom);

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
};

