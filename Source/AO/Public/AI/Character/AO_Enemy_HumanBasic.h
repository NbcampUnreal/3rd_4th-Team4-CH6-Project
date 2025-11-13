// AO_Enemy_HumanBasic.h

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "AO_Enemy_HumanBasic.generated.h"

UCLASS()
class AO_API AAO_Enemy_HumanBasic : public ACharacter
{
	GENERATED_BODY()

public:
	AAO_Enemy_HumanBasic();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UAIPerceptionStimuliSourceComponent* StimuliSource;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float WalkSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float ChaseSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Target")
	AActor* CurrentTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Target")
	FVector LastKnownPlayerLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	bool bIsPlayerVisible;

public:
	UFUNCTION(BlueprintCallable, Category = "Detection")
	void OnPlayerSpotted(AActor* PlayerActor);

	UFUNCTION(BlueprintCallable, Category = "Detection")
	void OnPlayerLost();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void PerformAttack();
};