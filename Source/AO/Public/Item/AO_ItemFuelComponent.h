#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Train/GAS/AO_Fuel_AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AO_ItemFuelComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AO_API UAO_ItemFuelComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAO_ItemFuelComponent();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fuel")
	float AddFuelAmount = 50.f;
	
	UFUNCTION(BlueprintCallable)
	void ApplyFuel();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	UAbilitySystemComponent* OwnerASC;

	UPROPERTY()
	UAO_Fuel_AttributeSet* OwnerFuelAttributeSet;
};
