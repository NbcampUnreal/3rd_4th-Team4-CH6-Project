// AO_Enemy_Werewolf.h

#pragma once

#include "CoreMinimal.h"
#include "AI/Character/AO_Enemy_HumanoidBase.h"
#include "AO_Enemy_Werewolf.generated.h"

UCLASS()
class AO_API AAO_Enemy_Werewolf : public AAO_Enemy_HumanoidBase
{
	GENERATED_BODY()

public:
	AAO_Enemy_Werewolf();

protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(Replicated)
	bool bHasHowled = false;
	
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Howl")
	float HowlNotifyRadius = 1500.0f;

public:
	UFUNCTION(BlueprintCallable)
	bool HasHowled() const { return bHasHowled; }
	
	void PerformHowl();
	
	FORCEINLINE float GetHowlRadius() const { return HowlNotifyRadius; }
};
