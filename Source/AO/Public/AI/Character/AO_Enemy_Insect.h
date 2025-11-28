// KSJ : AO_Enemy_Insect.h

#pragma once

#include "CoreMinimal.h"
#include "AI/Character/AO_EnemyBase.h"
#include "AO_Enemy_Insect.generated.h"

/**
 * Insect 적 AI 캐릭터
 * 
 * - 플레이어를 납치하여 이동
 * - 납치 중 뒷걸음질 이동
 * - 납치 중 플레이어에게 주기적 데미지
 */
UCLASS()
class AO_API AAO_Enemy_Insect : public AAO_EnemyBase
{
	GENERATED_BODY()

public:
	AAO_Enemy_Insect();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/* --- Settings --- */
	
	/** 납치 범위 (근접 거리) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Kidnap")
	float KidnapRange = 200.0f;
	
	/** 납치 중 이동 속도 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Kidnap")
	float KidnapMoveSpeed = 400.0f;
	
	/** 플레이어를 붙일 소켓 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Kidnap")
	FName PickupSocketName = TEXT("PickupSocket");
	
	/** 납치 중 데미지 간격 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Kidnap")
	float KidnapDamageInterval = 2.0f;
	
	/** 납치 중 데미지량 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Kidnap")
	float KidnapDamageAmount = 10.0f;
	
	/** 납치 최대 지속 시간 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Kidnap")
	float MaxKidnapDuration = 10.0f;
	
	/** 넉백 강도 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Kidnap")
	float KnockbackStrength = 1500.0f;
	
	/** 납치 데미지 GameplayEffect 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TSubclassOf<class UGameplayEffect> KidnapDamageEffectClass;

	/* --- State --- */
	
	/** 현재 납치 중인 플레이어 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_KidnappedPlayer, Category = "Kidnap")
	TObjectPtr<AActor> KidnappedPlayer;
	
	/** 납치 중인지 여부 */
	UPROPERTY(BlueprintReadOnly, Category = "Kidnap")
	bool bIsKidnapping = false;

	/* --- Actions --- */
	
	/** 플레이어 납치 시작 */
	UFUNCTION(BlueprintCallable, Category = "Kidnap")
	void StartKidnap(AActor* PlayerActor);
	
	/** 플레이어 납치 종료 (떨어뜨리기) */
	UFUNCTION(BlueprintCallable, Category = "Kidnap")
	void EndKidnap(bool bApplyKnockback = true);
	
	/** 뒷걸음질 이동 모드 설정 */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetBackwardMovement(bool bIsBackward);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;

private:
	/** 납치 시작 위치 */
	FVector KidnapStartLocation;
	
	/** 납치 시작 시간 */
	double KidnapStartTime = 0.0;
	
	/** 데미지 적용 타이머 */
	FTimerHandle KidnapDamageTimerHandle;
	
	/** 납치 중 데미지 적용 */
	void ApplyKidnapDamage();
	
	/** 납치 종료 처리 (내부) */
	void InternalEndKidnap(bool bApplyKnockback);
	
	/** KidnappedPlayer Replication 콜백 (클라이언트에서 Attach 처리) */
	UFUNCTION()
	void OnRep_KidnappedPlayer();
};
