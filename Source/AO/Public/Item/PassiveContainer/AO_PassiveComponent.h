#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "AO_PassiveComponent.generated.h"

struct FGameplayEventData;
class UAbilitySystemComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AO_API UAO_PassiveComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAO_PassiveComponent();

protected:
	virtual void BeginPlay() override;

	// Passive Effect
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Passive")
	TSubclassOf<UGameplayEffect> PassiveEffectClass;

	/*// 이벤트 수신 함수
	UFUNCTION()
	void OnGameplayEventReceived(const FGameplayEventData* Payload);
	*/

};
