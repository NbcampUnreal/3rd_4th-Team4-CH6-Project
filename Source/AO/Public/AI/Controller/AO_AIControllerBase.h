//KSJ : AO_AIControllerBase

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "AO_AIControllerBase.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;
class UStateTree;
class UStateTreeAIComponent;
class AAO_PlayerCharacter;
class UNavigationSystemV1;

/**
 * 모든 AI Controller의 공통 베이스 클래스
 * - AI Perception (시야, 청각)
 * - State Tree 통합
 * - 플레이어 감지 이벤트
 */
UCLASS()
class AO_API AAO_AIControllerBase : public AAIController
{
	GENERATED_BODY()

public:
	AAO_AIControllerBase();

	// IGenericTeamAgentInterface (AAIController가 이미 상속함, 오버라이드만 필요)
	virtual FGenericTeamId GetGenericTeamId() const override { return TeamId; }
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	// Perception 이벤트
	UFUNCTION()
	virtual void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	// 자식 클래스에서 오버라이드 가능한 이벤트
	virtual void OnPlayerDetected(AAO_PlayerCharacter* Player, const FVector& Location);
	virtual void OnPlayerLost(AAO_PlayerCharacter* Player, const FVector& LastKnownLocation);
	virtual void OnNoiseHeard(AActor* NoiseInstigator, const FVector& Location, float Volume);
	
	// 아이템 감지 이벤트 (자식 클래스에서 필요 시 오버라이드)
	virtual void OnActorDetected(AActor* Actor, const FVector& Location);
	virtual void OnActorLost(AActor* Actor);

	// Perception 설정
	void SetupPerceptionSystem();

public:
	// 현재 시야 내 플레이어 목록
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Perception")
	TArray<AAO_PlayerCharacter*> GetPlayersInSight() const;

	// 가장 가까운 플레이어
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Perception")
	AAO_PlayerCharacter* GetNearestPlayerInSight() const;

	// 시야 내 플레이어 존재 여부
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Perception")
	bool HasPlayerInSight() const;

	// NavMesh 도달 가능성 검증 함수들
	// 플레이어가 NavMesh 위에 있는지 확인 (AI가 도달 가능한 위치인지)
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Navigation")
	bool IsPlayerOnNavMesh(const AAO_PlayerCharacter* Player) const;

	// 플레이어까지 실제 경로가 존재하는지 확인 (더 정확하지만 비용이 높음)
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Navigation")
	bool CanReachPlayer(const AAO_PlayerCharacter* Player) const;

protected:
	// Perception
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|AI|Perception")
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|AI|Perception")
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|AI|Perception")
	TObjectPtr<UAISenseConfig_Hearing> HearingConfig;

	// State Tree
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|AI|StateTree")
	TObjectPtr<UStateTreeAIComponent> StateTreeComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|AI|StateTree")
	TObjectPtr<UStateTree> DefaultStateTree;

	// Perception 설정값 (에디터에서 수정 가능)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AO|AI|Perception|Sight")
	float SightRadius = 1500.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AO|AI|Perception|Sight")
	float LoseSightRadius = 2000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AO|AI|Perception|Sight")
	float PeripheralVisionAngleDegrees = 90.f;

	// 죽은 플레이어도 타겟팅할 것인지 여부 (기본값 false)
	// Insect 같이 시체를 이용하는 AI는 true로 설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AO|AI|Perception")
	bool bCanTargetDeadPlayer = false;

	// NavMesh 도달 불가능한 플레이어 필터링 여부 (기본값 true)
	// NavMesh 경계에서 AI가 멈추는 문제 방지
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AO|AI|Navigation")
	bool bFilterUnreachablePlayers = true;

	// NavMesh 검색 범위 (플레이어 위치에서 NavMesh를 찾는 범위)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AO|AI|Navigation")
	FVector NavMeshProjectionExtent = FVector(100.f, 100.f, 250.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AO|AI|Perception|Hearing")
	float HearingRange = 2000.f;

	// 현재 시야 내 플레이어들
	UPROPERTY()
	TArray<TWeakObjectPtr<AAO_PlayerCharacter>> PlayersInSight;

	// Team ID (AI는 1번 팀, 플레이어는 0번 팀으로 구분)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AO|AI|Team")
	FGenericTeamId TeamId = FGenericTeamId(1);
};
