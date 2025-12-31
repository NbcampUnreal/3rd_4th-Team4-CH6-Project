// HSJ : AO_DestructibleFloor.h
#pragma once

#include "CoreMinimal.h"
#include "Puzzle/Destructible/AO_DestructibleCacheActor.h"
#include "AO_DestructibleFloor.generated.h"

class UBoxComponent;

/**
 * 플레이어가 밟으면 n초 후 파괴되는 바닥
 * 
 * - 플레이어가 오버랩하면 DestructionDelay초 후 파괴 시작
 * - 파괴 후 CollisionDisableDelay초 후 바닥 Collision 제거 (플레이어가 떨어지는 시점)
 */
UCLASS()
class AO_API AAO_DestructibleFloor : public AAO_DestructibleCacheActor
{
	GENERATED_BODY()

public:
	AAO_DestructibleFloor();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void OnFloorBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	void StartDestructionTimer();
	void DisableFloorCollision();

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UBoxComponent> FloorCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UBoxComponent> OverlapTrigger;

	// 밟은 후 n초 뒤 파괴
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destruction|Timing", meta=(ClampMin="0.0"))
	float DestructionDelay = 1.0f;  

	// 파괴 후 m초 뒤 투명 바닥 제거
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destruction|Timing", meta=(ClampMin="0.0"))
	float CollisionDisableDelay = 0.5f;  

private:
	FTimerHandle DestructionTimerHandle;
	FTimerHandle CollisionDisableTimerHandle;
    
	bool bDestructionStarted = false;
};