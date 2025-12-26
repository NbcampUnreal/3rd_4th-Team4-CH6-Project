// HSJ : AO_StunDamageVolume.h
#pragma once

#include "CoreMinimal.h"
#include "Maps/Volumes/AO_DamageVolume.h"
#include "GameplayTagContainer.h"
#include "AO_StunDamageVolume.generated.h"

UCLASS()
class AO_API AAO_StunDamageVolume : public AAO_DamageVolume
{
	GENERATED_BODY()

public:
	AAO_StunDamageVolume();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	virtual void OnBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult) override;

	virtual void OnEndOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex) override;

private:
	void StartStunTimer(AActor* Enemy);
	void StopStunTimer(AActor* Enemy);

public:
	UPROPERTY(EditAnywhere, Category = "Stun")
	FGameplayTag StunEventTag;

	UPROPERTY(EditAnywhere, Category = "Stun", meta=(ClampMin="0.1", UIMin="0.1", UIMax="5.0"))
	float StunInterval = 1.0f;

private:
	TMap<TWeakObjectPtr<AActor>, FTimerHandle> StunTimerHandles;
};