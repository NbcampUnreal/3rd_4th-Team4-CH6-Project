#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AO_InventorySaveZone.generated.h"

class UBoxComponent;

UCLASS(ClassGroup=(Trigger), meta=(BlueprintSpawnableComponent))
class AO_API AAO_InventorySaveZone : public AActor
{
	GENERATED_BODY()

public:
	AAO_InventorySaveZone();

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> Zone;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnEnterSafeZone(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 BodyIndex,
		bool bFromSweep,
		const FHitResult& Hit);

	UFUNCTION()
	void OnExitSafeZone(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 BodyIndex);
};
