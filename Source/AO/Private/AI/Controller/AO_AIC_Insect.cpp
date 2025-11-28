// KSJ : AO_AIC_Insect.cpp

#include "AI/Controller/AO_AIC_Insect.h"
#include "AI/Character/AO_Enemy_Insect.h"
#include "AI/Manager/AO_Manager_InsectKidnapManager.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AO_Log.h"

AAO_AIC_Insect::AAO_AIC_Insect()
{
}

void AAO_AIC_Insect::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	AAO_Enemy_Insect* Insect = Cast<AAO_Enemy_Insect>(InPawn);
	if (Insect && GetBlackboardComponent())
	{
		AO_LOG(LogKSJ, Log, TEXT("[AIC_Insect] Possessed: %s"), *InPawn->GetName());
	}
}

void AAO_AIC_Insect::HandleSight(AActor* Actor, const FAIStimulus& Stimulus)
{
	UBlackboardComponent* BB = GetBlackboardComponent();
	if (!BB) return;
	
	AAO_Enemy_Insect* Insect = Cast<AAO_Enemy_Insect>(GetPawn());
	if (!Insect) return;
	
	if (Insect->bIsKidnapping)
	{
		return;
	}
	
	if (Stimulus.WasSuccessfullySensed())
	{
		UWorld* World = GetWorld();
		if (World)
		{
			if (UAO_Manager_InsectKidnapManager* Manager = World->GetSubsystem<UAO_Manager_InsectKidnapManager>())
			{
				if (!Manager->CanKidnapPlayer(Actor))
				{
					AO_LOG(LogKSJ, Verbose, TEXT("[AIC_Insect] Ignoring player %s (cooldown or already kidnapped)"), *Actor->GetName());
					return;
				}
			}
		}
		
		AActor* CurrentTarget = Cast<AActor>(BB->GetValueAsObject(BBKey_TargetPlayer));
		
		if (!CurrentTarget)
		{
			BB->SetValueAsObject(BBKey_TargetPlayer, Actor);
			BB->SetValueAsBool(BBKey_bPlayerVisible, true);
			BB->SetValueAsVector(BBKey_LastKnownPlayerLocation, Stimulus.StimulusLocation);
			AO_LOG(LogKSJ, Verbose, TEXT("[AIC_Insect] New target set: %s"), *Actor->GetName());
			return;
		}
		
		if (CurrentTarget == Actor)
		{
			if (World)
			{
				if (UAO_Manager_InsectKidnapManager* Manager = World->GetSubsystem<UAO_Manager_InsectKidnapManager>())
				{
					if (!Manager->CanKidnapPlayer(CurrentTarget))
					{
						BB->ClearValue(BBKey_TargetPlayer);
						BB->SetValueAsBool(BBKey_bPlayerVisible, false);
						AO_LOG(LogKSJ, Log, TEXT("[AIC_Insect] Current target %s is on cooldown, clearing target"), *CurrentTarget->GetName());
						return;
					}
				}
			}
			
			BB->SetValueAsBool(BBKey_bPlayerVisible, true);
			BB->SetValueAsVector(BBKey_LastKnownPlayerLocation, Stimulus.StimulusLocation);
			return;
		}
		
		bool bCurrentTargetVisible = BB->GetValueAsBool(BBKey_bPlayerVisible);
		if (bCurrentTargetVisible)
		{
			AO_LOG(LogKSJ, Verbose, TEXT("[AIC_Insect] Ignoring new target %s (already chasing %s)"), 
				*Actor->GetName(), *CurrentTarget->GetName());
			return;
		}
		
		BB->SetValueAsObject(BBKey_TargetPlayer, Actor);
		BB->SetValueAsBool(BBKey_bPlayerVisible, true);
		BB->SetValueAsVector(BBKey_LastKnownPlayerLocation, Stimulus.StimulusLocation);
		AO_LOG(LogKSJ, Log, TEXT("[AIC_Insect] Target switched from %s to %s"), 
			*CurrentTarget->GetName(), *Actor->GetName());
	}
	else
	{
		AActor* CurrentTarget = Cast<AActor>(BB->GetValueAsObject(BBKey_TargetPlayer));
		if (CurrentTarget == Actor)
		{
			BB->SetValueAsBool(BBKey_bPlayerVisible, false);
			BB->SetValueAsVector(BBKey_LastKnownPlayerLocation, Stimulus.StimulusLocation);
		}
	}
}
