// HSJ : AO_BallTrigger.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "AO_BallTrigger.generated.h"

class AAO_RollingBall;
class UBoxComponent;

UENUM(BlueprintType)
enum class EBallTriggerType : uint8
{
	Reset           UMETA(DisplayName = "Reset"),
	PlayerDetection UMETA(DisplayName = "PlayerDetection")
};

UCLASS()
class AO_API AAO_BallTrigger : public AActor
{
	GENERATED_BODY()

public:
	AAO_BallTrigger();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnTriggerBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnTriggerEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	int32 GetAlivePlayerCount() const;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BallTrigger")
	EBallTriggerType TriggerType = EBallTriggerType::PlayerDetection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BallTrigger")
	TArray<TObjectPtr<AAO_RollingBall>> TargetBalls;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BallTrigger")
	float AliveCheckInterval = 1.0f;

private:
	TArray<TWeakObjectPtr<AActor>> OverlappingPlayers;
	FTimerHandle AliveCheckTimerHandle;
	FGameplayTag DeathTag;
};