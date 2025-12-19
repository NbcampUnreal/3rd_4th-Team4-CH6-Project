#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayEffect.h"
#include "AO_PassiveComponent.generated.h"

struct FGameplayEventData;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AO_API UAO_PassiveComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAO_PassiveComponent();

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Passive")
	TSubclassOf<UGameplayEffect> MaxHpPassive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Passive")
	TSubclassOf<UGameplayEffect> MaxStaminaPassive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Passive")
	TSubclassOf<UGameplayEffect> MovementPassive;
	
	void OnGameplayEventReceived(const FGameplayEventData* Payload);
};
