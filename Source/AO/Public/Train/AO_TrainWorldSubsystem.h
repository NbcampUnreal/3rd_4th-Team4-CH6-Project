#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "AO_TrainWorldSubsystem.generated.h"

class UAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnTrainASCRegistered,
	UAbilitySystemComponent*, TrainASC
);

UCLASS()
class UAO_TrainWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	void RegisterTrainASC(UAbilitySystemComponent* InASC);
	
	UFUNCTION(BlueprintCallable, Category="Train")
	UAbilitySystemComponent* GetTrainASC() const;
	
	UFUNCTION(BlueprintCallable, Category="Train")
	void BindAndNotifyTrainASC(UObject* Listener);

	UPROPERTY(BlueprintAssignable, Category="Train")
	FOnTrainASCRegistered OnTrainASCRegistered;

	UFUNCTION(BlueprintCallable, Category="Train")
	void NotifyIfTrainASCReady();

private:
	UPROPERTY()
	UAbilitySystemComponent* CachedTrainASC = nullptr;
};
