// AO_Bull.h

#pragma once

#include "CoreMinimal.h"
#include "AI/Base/AO_AggressiveAIBase.h"
#include "AO_Bull.generated.h"

class UBoxComponent;

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

	// 돌진 충돌 처리 (돌진 중 Overlap 발생 시)
	UFUNCTION()
	void OnChargeOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
	virtual void BeginPlay() override;
	
	// 기절 처리 오버라이드
	virtual void HandleStunBegin() override;
	virtual void HandleStunEnd() override;

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
};

