//KSJ : AO_Stalker

#pragma once

#include "CoreMinimal.h"
#include "AI/Base/AO_AggressiveAIBase.h"
#include "AO_Stalker.generated.h"

class UAO_CeilingMoveComponent;

/**
 * Stalker (추적자형) AI 캐릭터
 * 
 * 특징:
 * - 사족보행, 천장 이동 가능
 * - 은신 및 기습 (엄폐물 활용)
 * - 치고 빠지기 (Hit & Run)
 */
UCLASS()
class AO_API AAO_Stalker : public AAO_AggressiveAIBase
{
	GENERATED_BODY()

public:
	AAO_Stalker();

	UFUNCTION(BlueprintCallable, Category = "AO|AI|Stalker")
	UAO_CeilingMoveComponent* GetCeilingMoveComponent() const { return CeilingMoveComp; }

	// 천장 이동 모드 전환
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Stalker")
	void SetCeilingMode(bool bEnable);

	// 천장/바닥 전환 몽타주 재생 및 모드 변경
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Stalker")
	void PlayCeilingTransitionMontage(bool bToCeiling);

	// 공격 후 후퇴 모드 설정
	void SetRetreatMode(bool bRetreat);
	bool IsRetreating() const { return bIsRetreating; }

	// 공격 상태 설정
	void SetIsAttacking(bool bAttacking) { bIsAttacking = bAttacking; }
	bool IsAttacking() const { return bIsAttacking; }

	// 전환 중인지 여부 확인
	bool IsTransitioningCeiling() const { return bIsTransitioningCeiling; }

protected:
	virtual void BeginPlay() override;
	virtual void HandleStunBegin() override;
	virtual void HandleStunEnd() override;

	UFUNCTION()
	void OnCeilingTransitionMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|AI|Stalker")
	TObjectPtr<UAO_CeilingMoveComponent> CeilingMoveComp;

	UPROPERTY(BlueprintReadOnly, Category = "AO|AI|Stalker")
	bool bIsRetreating = false;
	

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> JumpToCeilingMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> JumpToFloorMontage;

	// 전환 중 플래그
	bool bIsTransitioningCeiling = false;
	
	// 전환 목표 모드
	bool bPendingCeilingMode = false;
};

