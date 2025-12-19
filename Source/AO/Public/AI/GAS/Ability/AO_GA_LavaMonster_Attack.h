// AO_GA_LavaMonster_Attack.h

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AI/Character/AO_LavaMonster.h"
#include "AO_GA_LavaMonster_Attack.generated.h"

class AAO_PlayerCharacter;

/**
 * 땅속 공격 타겟 정보
 */
USTRUCT()
struct FAO_GroundStrikeTarget
{
	GENERATED_BODY()

	// 타겟 플레이어
	UPROPERTY()
	TWeakObjectPtr<AAO_PlayerCharacter> Player;

	// 공격 위치 (플레이어 발밑)
	UPROPERTY()
	FVector StrikeLocation = FVector::ZeroVector;

	// 전조 현상 시작 시간
	UPROPERTY()
	float WarningStartTime = 0.f;

	// 공격 발동 여부
	UPROPERTY()
	bool bHasStruck = false;
};

/**
 * 용암 몬스터 공격 전용 Gameplay Ability
 * 
 * 역할:
 * - 랜덤 공격 타입 선택 (Character에서 받음)
 * - 공격 타입별 몽타주 재생
 * - 공격 타입별 히트 판정
 *   - 근접/중거리/중~원거리: SphereTrace
 *   - 땅속 범위 공격: 범위 내 모든 플레이어 탐지 + 전조 + 데미지
 * - 데미지 및 넉백 적용
 * - HitReact 이벤트 발송
 */
UCLASS()
class AO_API UAO_GA_LavaMonster_Attack : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UAO_GA_LavaMonster_Attack();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

	virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags,
		const FGameplayTagContainer* TargetTags,
		FGameplayTagContainer* OptionalRelevantTags) const override;

protected:
	// 히트 이벤트 처리
	UFUNCTION()
	void OnHitConfirmEvent(FGameplayEventData Payload);

	// 땅속 공격: 범위 내 모든 플레이어 탐지 및 전조 시작
	void StartGroundStrike();

	// 땅속 공격: 전조 현상 업데이트 (디버그)
	UFUNCTION()
	void UpdateGroundStrikeWarning();

	// 땅속 공격: 각 타겟 위치에 공격 발동
	void ExecuteGroundStrikeAtTarget(int32 TargetIndex);

	// 데미지 및 넉백 적용
	void ApplyDamageAndKnockback(AActor* TargetActor, const FAO_LavaMonsterAttackConfig& Config);

	// HitReact 이벤트 발송
	void SendHitReactEvent(AActor* TargetActor, AActor* InstigatorActor);

	// 몽타주 종료 콜백
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();

protected:
	// 현재 공격 설정
	FAO_LavaMonsterAttackConfig CurrentAttackConfig;

	// 현재 공격 타입
	ELavaMonsterAttackType CurrentAttackType = ELavaMonsterAttackType::MeleePunch;

	// 데미지 Effect 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|AI|LavaMonster")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	// HitReact 태그
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|AI|LavaMonster")
	FGameplayTag HitReactTag;

	// 트레이스 채널
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|AI|LavaMonster")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Pawn;

	// 땅속 공격 타겟 목록
	UPROPERTY()
	TArray<FAO_GroundStrikeTarget> GroundStrikeTargets;

	// 땅속 공격 업데이트 타이머 핸들
	FTimerHandle GroundStrikeUpdateTimerHandle;

	// 땅속 공격 타겟별 공격 타이머 핸들
	TMap<int32, FTimerHandle> GroundStrikeAttackTimers;
};

