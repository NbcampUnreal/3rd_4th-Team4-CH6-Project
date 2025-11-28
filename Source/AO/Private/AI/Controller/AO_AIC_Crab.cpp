// KSJ : AO_AIC_Crab.cpp

#include "AI/Controller/AO_AIC_Crab.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Perception/AIPerceptionSystem.h"
#include "AO_Log.h"

AAO_AIC_Crab::AAO_AIC_Crab()
{
	if (SightConfig)
	{
		SightConfig->SightRadius = 1000.0f;
		SightConfig->LoseSightRadius = 1200.0f;
		SightConfig->PeripheralVisionAngleDegrees = 120.0f;
		SightConfig->SetMaxAge(10.0f);
	}
}

void AAO_AIC_Crab::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void AAO_AIC_Crab::HandleSight(AActor* Actor, const FAIStimulus& Stimulus)
{
	Super::HandleSight(Actor, Stimulus);
}