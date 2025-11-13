// AO_AIC_HumanoidBase.h

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "AO_AIC_HumanoidBase.generated.h"

class UBlackboardComponent;
class UAIPerceptionComponent;
class UAISenseConfig_Sight;

UCLASS()
class AO_API AAO_AIC_HumanoidBase : public AAIController
{
	GENERATED_BODY()

public:
	AAO_AIC_HumanoidBase();

protected:
	virtual void BeginPlay() override;
	

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

protected:
	UPROPERTY(VisibleAnywhere, Category = "AI")
	TObjectPtr<UBlackboardComponent> BlackboardComp;

public:
	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	UPROPERTY()
	class AAO_Enemy_HumanoidBase* ControlledHumanoid = nullptr;

protected:
	FName Key_TargetActor = TEXT("TargetActor");
	FName Key_LastSeenLocation = TEXT("LastSeenLocation");
	FName Key_HasLineOfSight = TEXT("HasLineOfSight");
};