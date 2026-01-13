//KSJ : AO_Bull

#pragma once

#include "CoreMinimal.h"
#include "AI/Base/AO_AggressiveAIBase.h"
#include "AO_Bull.generated.h"

class UBoxComponent;
class AAO_AggressiveAICtrl;

/**
 * Bull (황소형) AI 캐릭터
 * 
 * 특징:
 * - 플레이어를 발견하면 돌진(Charge) 공격
 * - 돌진 중 충돌하면 플레이어 넉백 & 넉다운
 * - 기본 속도: 300, 추격 속도: 600 (예정)
 */
UCLASS()
class AO_API AAO_Bull : public AAO_AggressiveAIBase
{
	GENERATED_BODY()

public:
	AAO_Bull();

	// 돌진 중인지 여부
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Bull")
	bool IsCharging() const { return bIsCharging; }

	// 돌진 상태 설정 (GAS 또는 Task에서 호출)
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Bull")
	void SetIsCharging(bool bCharging);

	// 공격 후 쿨다운 상태인지 확인 (부모 클래스 오버라이드)
	virtual bool IsInPostAttackCooldown() const override { return bInPostAttackCooldown; }

	// 공격 후 후퇴 시작 (Ability 종료 시 호출)
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Bull")
	void StartPostAttackRetreat();

	// 돌진 충돌 처리 (돌진 중 Overlap 발생 시)
	UFUNCTION()
	void OnChargeOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// 공통 공격 인터페이스 오버라이드
	virtual FEnemyAttackConfig GetCurrentAttackConfig_Implementation() const override;

protected:
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
	
	// 기절 처리 오버라이드
	virtual void HandleStunBegin() override;
	virtual void HandleStunEnd() override;

	// 후퇴 완료 후 대기 시작
	void OnRetreatComplete();

	// 쿨다운 종료
	void EndPostAttackCooldown();

protected:
	// 돌진 충돌 판정용 박스 (머리 부분)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|AI|Bull")
	TObjectPtr<UBoxComponent> ChargeCollisionBox;

	// 돌진 상태 플래그
	UPROPERTY(BlueprintReadOnly, Category = "AO|AI|Bull")
	bool bIsCharging = false;

	// 돌진 데미지
	UPROPERTY(EditDefaultsOnly, Category = "AO|AI|Bull|Combat")
	float ChargeDamage = 30.f;

	// 돌진 넉백 강도
	UPROPERTY(EditDefaultsOnly, Category = "AO|AI|Bull|Combat")
	float KnockbackStrength = 1000.f;

	// 돌진 속도
	UPROPERTY(EditDefaultsOnly, Category = "AO|AI|Bull|Movement")
	float ChargeSpeed = 900.f;

	// 데미지 Effect 클래스 (OnChargeOverlap에서 사용)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|AI|Bull|Combat")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	// 근접 공격 설정 (신규 시스템)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|AI|Bull|Combat")
	FEnemyAttackConfig MeleeAttackConfig;

	// === 공격 후 후퇴/쿨다운 시스템 ===
	
	// 공격 후 후퇴 거리 (에디터에서 조절 가능)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AO|AI|Bull|PostAttack")
	float RetreatDistance = 400.f;

	// 후퇴 후 대기 시간 (플레이어가 일어나는 시간보다 약간 길게)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AO|AI|Bull|PostAttack")
	float PostAttackWaitTime = 3.5f;

	// 공격 후 쿨다운 상태 플래그
	UPROPERTY(BlueprintReadOnly, Category = "AO|AI|Bull")
	bool bInPostAttackCooldown = false;

	// 후퇴 중 플래그
	UPROPERTY(BlueprintReadOnly, Category = "AO|AI|Bull")
	bool bIsRetreating = false;

	// 쿨다운 타이머 핸들
	FTimerHandle PostAttackTimerHandle;

	// 후퇴 체크 타이머 핸들
	FTimerHandle RetreatCheckTimerHandle;

	// 후퇴 목표 위치
	FVector RetreatTargetLocation;
};

