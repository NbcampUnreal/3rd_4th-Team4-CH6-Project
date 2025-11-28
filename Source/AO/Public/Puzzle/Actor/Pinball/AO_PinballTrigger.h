// HSJ : AO_PinballTrigger.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "AO_PinballTrigger.generated.h"

class AAO_PinballBall;
class AAO_PuzzleConditionChecker;
class UBoxComponent;

/**
 * 핀볼 오버랩 영역
 * - Success: 핀볼 공이 성공 영역에 가면 체커에 성공 태그 전달
 * - Fail: 핀볼 공이 실패 영역에 가면 초기 위치로 이동
 */

UENUM(BlueprintType)
enum class EPinballTriggerMode : uint8
{
	Success,
	Fail
};

UCLASS()
class AO_API AAO_PinballTrigger : public AActor
{
	GENERATED_BODY()

public:
	AAO_PinballTrigger();

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:
	UPROPERTY(EditAnywhere, Category="Pinball")
	EPinballTriggerMode TriggerMode = EPinballTriggerMode::Success;

	UPROPERTY(EditAnywhere, Category="Pinball")
	TObjectPtr<AAO_PinballBall> PinballBall;

	UPROPERTY(EditAnywhere, Category="Pinball")
	TObjectPtr<AAO_PuzzleConditionChecker> LinkedChecker;

	UPROPERTY(EditAnywhere, Category="Pinball")
	FGameplayTag EventTag;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> TriggerBox;
};