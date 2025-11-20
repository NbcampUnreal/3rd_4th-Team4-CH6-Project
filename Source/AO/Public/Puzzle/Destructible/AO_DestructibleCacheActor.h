// HSJ : AO_DestructibleCacheActor.h
#pragma once

#include "CoreMinimal.h"
#include "Chaos/CacheManagerActor.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "AO_DestructibleCacheActor.generated.h"

class UAbilitySystemComponent;
class UGeometryCollectionComponent;

/**
 * 카오스 캐시를 사용한 파괴 가능한 오브젝트
 * 
 * - GameplayTag 이벤트로 파괴 트리거
 * - 카오스 캐시 시스템 통합 (미리 녹화된 파괴 재생)
 * 
 * 1. TriggerTag가 ASC에 추가되면 자동 파괴
 * 2. TriggerDestruction() 직접 호출로도 파괴 가능
 * 3. 서버에서 파괴 실행 → bIsDestroyed 복제 → 클라이언트에서도 파괴
 * 
 * 사용 예:
 * - 퍼즐 해결 시 벽 부서짐
 * - 폭발물로 장애물 제거
 */
UCLASS()
class AO_API AAO_DestructibleCacheActor : public AChaosCacheManager, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AAO_DestructibleCacheActor();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// 파괴 트리거
	UFUNCTION(BlueprintCallable, Category="Destruction")
	void TriggerDestruction();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destruction")
	TObjectPtr<UGeometryCollectionComponent> GeoComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Abilities")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destruction")
	FGameplayTag TriggerTag; // 이 태그가 추가되면 파괴 실행

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destruction", meta=(ClampMin="0.0"))
	float DestroyDelay = 4.0f;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UFUNCTION(Server, Reliable)
	void ServerTriggerDestruction();

	// GameplayTag 변경 콜백
	void OnTagChanged(const FGameplayTag Tag, int32 NewCount);

	// 실제 파괴 실행 (카오스 캐시 재생)
	void ExecuteDestruction();

	UFUNCTION()
	void OnRep_IsDestroyed();

	UPROPERTY(ReplicatedUsing=OnRep_IsDestroyed)
	bool bIsDestroyed = false;

	FTimerHandle DestroyTimerHandle;
};