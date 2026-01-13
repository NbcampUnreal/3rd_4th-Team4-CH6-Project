//KSJ : AO_PackCoordComp

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AO_PackCoordComp.generated.h"

class AAO_Werewolf;
class AAO_PlayerCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHowlReceived, AActor*, TargetActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCoordinatedAttackStarted);

/**
 * Howl 실행 결과
 */
USTRUCT(BlueprintType)
struct FAO_HowlResult
{
	GENERATED_BODY()

	// 동료를 찾았는지 여부
	UPROPERTY(BlueprintReadOnly)
	bool bFoundAlly = false;

	// 발견한 동료 수
	UPROPERTY(BlueprintReadOnly)
	int32 AllyCount = 0;
};

/**
 * 포위 위치 예약 정보
 */
USTRUCT()
struct FAO_SurroundPosition
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location = FVector::ZeroVector;

	UPROPERTY()
	TWeakObjectPtr<AAO_Werewolf> ReservedBy;

	UPROPERTY()
	float ReservationTime = 0.f;
};

/**
 * 도주로 예약 정보
 */
USTRUCT()
struct FAO_EscapeRouteReservation
{
	GENERATED_BODY()

	UPROPERTY()
	FVector RouteLocation = FVector::ZeroVector;

	UPROPERTY()
	TWeakObjectPtr<AAO_Werewolf> ReservedBy;

	UPROPERTY()
	float ReservationTime = 0.f;

	UPROPERTY()
	int32 RouteIndex = -1;
};

/**
 * Werewolf 무리(Pack) 행동을 관리하는 컴포넌트
 * - Howl 전파 및 수신
 * - 무리 멤버 관리 (EQS Context용)
 * - 포위/공격 상태 동기화
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class AO_API UAO_PackCoordComp : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAO_PackCoordComp();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// --- Howl Logic ---
	
	// 하울링 시작 (주변 아군에게 전파) - 결과 반환
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Werewolf")
	FAO_HowlResult BroadcastHowl(AActor* Target);

	// 하울링 수신 (다른 아군으로부터)
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Werewolf")
	void ReceiveHowl(AActor* Sender, AActor* Target);

	// 하울링 수신 이벤트 (StateTree 등에서 바인딩)
	UPROPERTY(BlueprintAssignable, Category = "AO|AI|Werewolf")
	FOnHowlReceived OnHowlReceived;

	// 일제공격 시작 이벤트
	UPROPERTY(BlueprintAssignable, Category = "AO|AI|Werewolf")
	FOnCoordinatedAttackStarted OnCoordinatedAttackStarted;

	// 하울링 범위
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AO|AI|Werewolf")
	float HowlRadius = 2000.f;

	// Howl 실행자 여부
	UFUNCTION(BlueprintPure, Category = "AO|AI|Werewolf")
	bool IsHowlInitiator() const { return bIsHowlInitiator; }

	// --- Pack Management ---

	// 주변의 모든 Werewolf 찾기 (자신 제외)
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Werewolf")
	TArray<AAO_Werewolf*> GetNearbyPackMembers() const;

	// 현재 포위 모드인지?
	bool IsSurrounding() const { return bIsSurrounding; }
	
	// 포위 모드 설정
	void SetSurroundMode(bool bSurround);

	// 일제 공격 시작 (타이머 종료 or 준비 완료)
	void StartCoordinatedAttack();

	// 일제공격 강제 시작 (타이머 무시)
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Werewolf")
	void ForceStartCoordinatedAttack();

	// 일제공격이 시작되었는지 확인
	UFUNCTION(BlueprintPure, Category = "AO|AI|Werewolf")
	bool IsCoordinatedAttackStarted() const { return bIsCoordinatedAttackStarted; }

	// --- 포위 위치 관리 ---

	// 포위 위치 예약
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Werewolf")
	bool TryReserveSurroundPosition(const FVector& Location, AAO_Werewolf* Reserver, float ReservationTimeout = 10.f);

	// 포위 위치 해제
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Werewolf")
	void ReleaseSurroundPosition(AAO_Werewolf* Reserver);

	// 사용 가능한 포위 위치 찾기 (다른 Werewolf 위치 고려)
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Werewolf")
	FVector FindAvailableSurroundPosition(AAO_PlayerCharacter* Target, float Radius, AAO_Werewolf* ExcludeWolf, float MinDistanceFromOthers = 300.f);

	// 포위 위치 도달 여부 체크
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Werewolf")
	void CheckSurroundPositionReached(const FVector& CurrentLocation, float AcceptanceRadius = 150.f);

	// 포위 위치 도달 여부
	UFUNCTION(BlueprintPure, Category = "AO|AI|Werewolf")
	bool HasReachedSurroundPosition() const { return bHasReachedSurroundPosition; }

	// 목표 포위 위치 설정
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Werewolf")
	void SetTargetSurroundPosition(const FVector& Position) { TargetSurroundPosition = Position; }

	// --- 도주로 차단 관리 ---

	// 각 Werewolf에게 도주로 할당
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Werewolf")
	int32 AssignEscapeRouteToWolf(AAO_PlayerCharacter* Target, AAO_Werewolf* Wolf, float SearchRadius = 1000.f);

	// 할당된 도주로 차단 위치 가져오기
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Werewolf")
	FVector GetAssignedEscapeRouteBlockPosition(AAO_Werewolf* Wolf, AAO_PlayerCharacter* Target, float BlockRadius = 500.f);

	// 도주로 예약 해제
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Werewolf")
	void ReleaseEscapeRoute(AAO_Werewolf* Wolf);

	// 도주 경로 캐시 가져오기 (다른 Werewolf Controller에서 사용)
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Werewolf")
	TArray<FVector> GetCachedEscapeRoutes() const { return CachedEscapeRoutes; }

protected:
	// 현재 포위 중인지 여부
	bool bIsSurrounding = false;

	// 일제공격 시작 여부
	bool bIsCoordinatedAttackStarted = false;

	// Howl 실행자 여부
	bool bIsHowlInitiator = false;

	// 포위 위치 도달 여부
	bool bHasReachedSurroundPosition = false;

	// 목표 포위 위치
	FVector TargetSurroundPosition = FVector::ZeroVector;

	// 일제 공격 타이머 핸들
	FTimerHandle AttackTimerHandle;

	// 일제 공격까지 대기 시간 (5~8초)
	UPROPERTY(EditDefaultsOnly, Category = "AO|AI|Werewolf")
	float MinAttackDelay = 5.f;

	UPROPERTY(EditDefaultsOnly, Category = "AO|AI|Werewolf")
	float MaxAttackDelay = 8.f;

	// 포위 위치 예약 목록
	UPROPERTY()
	TArray<FAO_SurroundPosition> ReservedPositions;

	// 포위 위치 예약 만료 시간
	UPROPERTY(EditDefaultsOnly, Category = "AO|AI|Werewolf")
	float PositionReservationTimeout = 10.f;

	// 도주로 예약 목록
	UPROPERTY()
	TArray<FAO_EscapeRouteReservation> ReservedEscapeRoutes;

	// 캐시된 도주 경로들 (모든 Werewolf가 공유)
	UPROPERTY()
	TArray<FVector> CachedEscapeRoutes;

	// 도주로 예약 만료 시간
	UPROPERTY(EditDefaultsOnly, Category = "AO|AI|Werewolf")
	float EscapeRouteReservationTimeout = 15.f;

	// 공격 명령 실행
	void ExecuteAttack();

	// 만료된 포위 위치 예약 정리
	void CleanupExpiredReservations();

	// 만료된 도주로 예약 정리
	void CleanupExpiredEscapeRouteReservations();
};
