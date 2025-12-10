#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AO_Fuel_AttributeSet.generated.h"

UCLASS()
class AO_API UAO_Fuel_AttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UAO_Fuel_AttributeSet();
	
	UPROPERTY(BlueprintReadOnly, Category="Fuel", ReplicatedUsing=OnRep_Fuel)
	FGameplayAttributeData Fuel;
	ATTRIBUTE_ACCESSORS_BASIC(UAO_Fuel_AttributeSet, Fuel)
	
protected:
	UFUNCTION()
	void OnRep_Fuel(const FGameplayAttributeData& OldFuel);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
