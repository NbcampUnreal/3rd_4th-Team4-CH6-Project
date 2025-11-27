// AO_DamageComponent.h

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Components/ActorComponent.h"
#include "AO_DamageComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AO_API UAO_DamageComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAO_DamageComponent();

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category = "GAS|Damage")
	void PerformMeleeDamage(
		TSubclassOf<UGameplayEffect> DamageEffectClass,
		float DamageAmount, FVector TraceStart,
		FVector TraceEnd,
		float TraceRadius = 25.f);

private:
	void ApplyDamageEffect(UAbilitySystemComponent* TargetASC, TSubclassOf<UGameplayEffect> DamageEffectClass, float DamageAmount);

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> OwnerASC;
};
