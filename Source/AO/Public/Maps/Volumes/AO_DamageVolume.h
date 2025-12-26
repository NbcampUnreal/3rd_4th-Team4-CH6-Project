// AO_DamageVolume.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemComponent.h"
#include "AO_DamageVolume.generated.h"

class UGameplayEffect;
class UBoxComponent;

UCLASS()
class AO_API AAO_DamageVolume : public AActor
{
	GENERATED_BODY()

public:
	AAO_DamageVolume();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UBoxComponent> BoxComponent;

	UPROPERTY(EditAnywhere, Category = "GAS")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditAnywhere, Category = "Damage")
	float DamagePerSecond = 10.f;

	TMap<TWeakObjectPtr<AActor>, FActiveGameplayEffectHandle> ActiveHandles;
	
	UFUNCTION()
	virtual void OnBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnEndOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);
};
