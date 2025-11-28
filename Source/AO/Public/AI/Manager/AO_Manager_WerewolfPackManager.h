// KSJ : AO_Manager_WerewolfPackManager.h

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "AO_Manager_WerewolfPackManager.generated.h"

class AAO_AIC_Werewolf;
class AActor;

/**
 * 웨어울프 무리(Pack)의 행동을 총괄하는 매니저 시스템 (WorldSubsystem)
 * 
 * [역할]
 * 1. 레벨 내 모든 웨어울프 AI 등록 및 관리
 * 2. Howl 이벤트 중계 (최초 발견 -> 주변 전파)
 * 3. 포위(Siege) 상태 및 일제 공격(Rush) 타이머 관리
 * 4. 타겟 플레이어에 대한 포위 점유 현황(Slot) 관리
 */

UENUM(BlueprintType)
enum class EWerewolfPackState : uint8
{
	Idle		UMETA(DisplayName = "Idle"),
	Siege		UMETA(DisplayName = "Siege"), // 포위 중 (도주로 차단)
	Rush		UMETA(DisplayName = "Rush")   // 일제 공격 (추적)
};

// 특정 타겟 플레이어에 대한 포위 정보를 담는 구조체
USTRUCT()
struct FPackSiegeInfo
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<AActor> TargetPlayer = nullptr;

	UPROPERTY()
	TArray<TObjectPtr<AAO_AIC_Werewolf>> AssignedWerewolves;

	UPROPERTY()
	FTimerHandle RushTimerHandle;

	double SiegeStartTime = 0.0;

	TArray<FVector> EscapeRouteDirections;
	TMap<TObjectPtr<AAO_AIC_Werewolf>, int32> SlotAssignment;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPackStateChanged, EWerewolfPackState, NewState);

UCLASS()
class AO_API UAO_Manager_WerewolfPackManager : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// --- Registration ---
	void RegisterWerewolf(AAO_AIC_Werewolf* WerewolfAIC);
	void UnregisterWerewolf(AAO_AIC_Werewolf* WerewolfAIC);

	// --- Howl & Siege Logic ---
	
	/** 
	 * [최초 발견자 -> 매니저] "내가 얘를 발견해서 하울링했다" 보고 
	 * @return true면 유효한 보고(최초), false면 이미 다른 늑대가 보고한 타겟
	 */
	bool ReportPlayerDetection(AAO_AIC_Werewolf* Howler, AActor* TargetActor);

	/** 현재 해당 액터가 이미 Pack의 타겟이 되어있는지 확인 */
	bool IsTargetAlreadyMarked(AActor* TargetActor) const;

	/** 특정 늑대에게 현재 상태(State) 반환 */
	EWerewolfPackState GetPackStateFor(AAO_AIC_Werewolf* RequestingWolf) const;

	/** 특정 타겟에 대한 포위 진형에서, 이 늑대가 맡아야 할 위치(도주로) 혹은 역할이 있는지? (EQS 쿼리용 정보 제공) */
	AActor* GetSiegeTargetFor(AAO_AIC_Werewolf* RequestingWolf) const;

	/** 포위 진형 업데이트 (도주로 재계산) - BT Service에서 주기적으로 호출 */
	void UpdateSiegeFormationFor(AAO_AIC_Werewolf* RequestingWolf);

	/** 특정 늑대에게 할당된 슬롯 방향 반환 (EQS용) */
	bool GetAssignedSlotDirection(AAO_AIC_Werewolf* RequestingWolf, FVector& OutDirection) const;

	/** 도주로 재계산 필요 여부 체크 및 실행 (플레이어 이동 대응) */
	void UpdateSiegeFormation(AActor* TargetActor);

	// --- Event Handling ---
	
	// 난입 등으로 인해 타겟이 변경되었을 때 호출
	void OnTargetSwitched(AAO_AIC_Werewolf* Wolf, AActor* OldTarget, AActor* NewTarget);

	/** 일제 공격(Rush) 시작 - 타이머 종료 시 호출 */
	void StartCoordinatedAttack(AActor* TargetActor);

private:
	UPROPERTY()
	TArray<TObjectPtr<AAO_AIC_Werewolf>> RegisteredWerewolves;

	UPROPERTY()
	TMap<TObjectPtr<AActor>, FPackSiegeInfo> ActiveSieges;

	/** 포위 -> 일제공격 전환 시간 (3~4초) */
	float SiegeDuration = 3.5f;

	/** Howl 전파 범위 (미터 단위, 필요시 조정) */
	float HowlPropagationRadius = 2000.0f;

	/** 플레이어 주변의 도주로(개방된 방향) 계산 */
	TArray<FVector> CalculateEscapeRoutes(AActor* TargetActor) const;

	/** 각 늑대에게 도주로 슬롯 할당 */
	void AssignSiegeSlots(AActor* TargetActor);

	/** 도주로 계산 시 사용할 각도 간격 (도) */
	UPROPERTY(EditDefaultsOnly, Category = "Siege")
	float EscapeRouteAngleStep = 30.0f; // 30도 간격

	/** 도주로 탐지 거리 (미터) */
	UPROPERTY(EditDefaultsOnly, Category = "Siege")
	float EscapeRouteCheckDistance = 1000.0f;

	/** 도주로 재계산 주기 (초) - 플레이어 이동 대응 */
	UPROPERTY(EditDefaultsOnly, Category = "Siege")
	float EscapeRouteRecalcInterval = 1.0f;

	TMap<TObjectPtr<AActor>, double> LastEscapeRouteCalcTime;
};
