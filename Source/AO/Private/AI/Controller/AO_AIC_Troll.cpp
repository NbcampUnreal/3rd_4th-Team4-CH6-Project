// KSJ : AO_AIC_Troll.cpp

#include "AI/Controller/AO_AIC_Troll.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AO_Log.h"

AAO_AIC_Troll::AAO_AIC_Troll()
{
	if (SightConfig)
	{
		SightConfig->SightRadius = 1500.0f;
		SightConfig->LoseSightRadius = 1800.0f;
		SightConfig->PeripheralVisionAngleDegrees = 90.0f;
		SightConfig->SetMaxAge(15.0f);
	}
}

void AAO_AIC_Troll::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void AAO_AIC_Troll::HandleSight(AActor* Actor, const FAIStimulus& Stimulus)
{
	if (!GetBlackboardComponent()) return;

	Super::HandleSight(Actor, Stimulus);

	if (Stimulus.WasSuccessfullySensed())
	{
		UObject* CurrentTargetObj = GetBlackboardComponent()->GetValueAsObject(TEXT("TargetPlayer"));
		AActor* CurrentTarget = Cast<AActor>(CurrentTargetObj);

		if (!CurrentTarget)
		{
			GetBlackboardComponent()->SetValueAsObject(TEXT("TargetPlayer"), Actor);
			GetBlackboardComponent()->SetValueAsBool(TEXT("bPlayerVisible"), true);
			return;
		}

		if (CurrentTarget != Actor)
		{
			APawn* MyPawn = GetPawn();
			if (MyPawn)
			{
				float DistToCurrent = FVector::DistSquared(MyPawn->GetActorLocation(), CurrentTarget->GetActorLocation());
				float DistToNew = FVector::DistSquared(MyPawn->GetActorLocation(), Actor->GetActorLocation());

				double CurrentTime = GetWorld()->GetTimeSeconds();
				if (DistToNew < DistToCurrent - (300.0f * 300.0f))
				{
					if (CurrentTime - LastTargetChangeTime > TargetChangeCooldown)
					{
						ConsiderNewTarget(Actor);
					}
				}
			}
		}
	}
}

void AAO_AIC_Troll::ConsiderNewTarget(AActor* NewTarget)
{
	if (!GetBlackboardComponent()) return;

	PotentialTarget = NewTarget;
	GetBlackboardComponent()->SetValueAsBool(TEXT("bIsHesitating"), true);
	GetBlackboardComponent()->SetValueAsObject(TEXT("PotentialTarget"), NewTarget);

	LastTargetChangeTime = GetWorld()->GetTimeSeconds();
	
	AO_LOG(LogKSJ, Log, TEXT("Troll is hesitating to switch target to %s"), *NewTarget->GetName());
}