// KSJ : AO_AIC_EnemyBase.h

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h" // 청각 추가
#include "GenericTeamAgentInterface.h"
#include "AO_AIC_EnemyBase.generated.h"

/**
 * 모든 적 AI Controller의 Base 클래스
 * 
 * - 공통 감각 설정 (시각 필수, 청각 옵션)
 * - 멀티플레이어 기억 시스템 (Memory Map)
 * - 팀 설정 (Team ID 1: Enemy)
 */
UCLASS()
class AO_API AAO_AIC_EnemyBase : public AAIController
{
	GENERATED_BODY()

public:
	AAO_AIC_EnemyBase();

	// IGenericTeamAgentInterface
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;

	// 기억된 모든 플레이어의 위치 반환 (EQS용)
	UFUNCTION(BlueprintCallable, Category = "AI|Memory")
	TArray<FVector> GetAllLastKnownLocations() const;

	// 특정 액터의 마지막 위치 반환
	UFUNCTION(BlueprintCallable, Category = "AI|Memory")
	bool GetLastKnownLocationOf(AActor* TargetActor, FVector& OutLocation) const;

protected:
	virtual void OnPossess(APawn* InPawn) override;

	/* --- Components --- */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UAISenseConfig_Hearing> HearingConfig; // 청각은 옵션으로 활성화

	/* --- Settings --- */
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTreeAsset;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FGenericTeamId TeamId;

	UPROPERTY()
	TMap<TObjectPtr<AActor>, FVector> PlayerMemoryMap;

	/* --- Events --- */
	UFUNCTION()
	virtual void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	// 파생 클래스에서 감각 처리 로직을 확장할 수 있도록 가상함수 제공
	virtual void HandleSight(AActor* Actor, const FAIStimulus& Stimulus);
	virtual void HandleHearing(AActor* Actor, const FAIStimulus& Stimulus);
};