// AO_AggressiveAIBase.h

#pragma once

#include "CoreMinimal.h"
#include "AI/Base/AO_AICharacterBase.h"
#include "AO_AggressiveAIBase.generated.h"

class AAO_PlayerCharacter;

/**
 * 선공형 몬스터 공통 베이스 클래스
 * 
 * 특징:
 * - 플레이어를 추격하고 공격하는 AI
 * - 배회 → 추격 → 수색 → 배회 사이클
 * - 공격 범위 및 속도 설정
 * - 충돌 회피 (서로의 경로를 막지 않음)
 */
UCLASS()
class AO_API AAO_AggressiveAIBase : public AAO_AICharacterBase
{
	GENERATED_BODY()

public:
	AAO_AggressiveAIBase();

	// 추격 모드 설정
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Aggressive")
	void SetChaseMode(bool bChasing);

	UFUNCTION(BlueprintCallable, Category = "AO|AI|Aggressive")
	bool IsInChaseMode() const { return bIsChasing; }

	// 현재 추격 대상
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Aggressive")
	AAO_PlayerCharacter* GetCurrentTarget() const { return CurrentTarget.Get(); }

	UFUNCTION(BlueprintCallable, Category = "AO|AI|Aggressive")
	void SetCurrentTarget(AAO_PlayerCharacter* NewTarget);

	// 공격 범위 확인
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Aggressive")
	virtual bool IsTargetInAttackRange() const;

	UFUNCTION(BlueprintCallable, Category = "AO|AI|Aggressive")
	float GetAttackRange() const { return AttackRange; }

	// 수색 모드
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Aggressive")
	void SetSearchMode(bool bSearching);

	UFUNCTION(BlueprintCallable, Category = "AO|AI|Aggressive")
	bool IsInSearchMode() const { return bIsSearching; }

	// 속도 설정
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Aggressive")
	float GetChaseSpeed() const { return ChaseSpeed; }

	UFUNCTION(BlueprintCallable, Category = "AO|AI|Aggressive")
	float GetRoamSpeed() const { return RoamSpeed; }

protected:
	virtual void BeginPlay() override;

	// 이동 속도 업데이트
	virtual void UpdateMovementSpeed();

protected:
	// 이동 속도 설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AO|AI|Aggressive|Movement")
	float RoamSpeed = 300.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AO|AI|Aggressive|Movement")
	float ChaseSpeed = 500.f;

	// 공격 관련 설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AO|AI|Aggressive|Combat")
	float AttackRange = 200.f;

	// 수색 관련 설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AO|AI|Aggressive|Search")
	float SearchDuration = 7.f;  // 5~10초 사이, 기본 7초

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AO|AI|Aggressive|Search")
	float SearchRadius = 500.f;

	// 현재 상태
	UPROPERTY(BlueprintReadOnly, Category = "AO|AI|Aggressive")
	bool bIsChasing = false;

	UPROPERTY(BlueprintReadOnly, Category = "AO|AI|Aggressive")
	bool bIsSearching = false;

	// 현재 추격 대상
	UPROPERTY()
	TWeakObjectPtr<AAO_PlayerCharacter> CurrentTarget;
};
