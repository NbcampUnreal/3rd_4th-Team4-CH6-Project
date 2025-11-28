// KSJ : AO_AIC_EnemyBase.cpp

#include "AI/Controller/AO_AIC_EnemyBase.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Perception/AIPerceptionSystem.h"
#include "AO_Log.h"

AAO_AIC_EnemyBase::AAO_AIC_EnemyBase()
{
	TeamId = FGenericTeamId(1); // Team 1: Enemy

	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComp"));
	
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 1500.0f;
	SightConfig->LoseSightRadius = 1800.0f;
	SightConfig->PeripheralVisionAngleDegrees = 120.0f;
	SightConfig->SetMaxAge(10.0f);
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = false; 

	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	HearingConfig->HearingRange = 2000.0f;
	HearingConfig->SetMaxAge(10.0f);
	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = false;

	AIPerceptionComp->ConfigureSense(*SightConfig);
	AIPerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());
	
	AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &AAO_AIC_EnemyBase::OnTargetPerceptionUpdated);
}

FGenericTeamId AAO_AIC_EnemyBase::GetGenericTeamId() const
{
	return TeamId;
}

ETeamAttitude::Type AAO_AIC_EnemyBase::GetTeamAttitudeTowards(const AActor& Other) const
{
	if (const APawn* OtherPawn = Cast<APawn>(&Other))
	{
		if (const IGenericTeamAgentInterface* TeamAgent = Cast<IGenericTeamAgentInterface>(OtherPawn->GetController()))
		{
			return (TeamAgent->GetGenericTeamId() == TeamId) ? ETeamAttitude::Friendly : ETeamAttitude::Hostile;
		}
	}
	return ETeamAttitude::Hostile;
}

void AAO_AIC_EnemyBase::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	if (HasAuthority() && BehaviorTreeAsset)
	{
		if (RunBehaviorTree(BehaviorTreeAsset))
		{
			if (GetBlackboardComponent())
			{
				GetBlackboardComponent()->SetValueAsObject(TEXT("SelfActor"), InPawn);
				AO_LOG(LogKSJ, Log, TEXT("AI Started: %s (Class: %s)"), *InPawn->GetName(), *GetName());
			}
		}
		else
		{
			AO_LOG(LogKSJ, Error, TEXT("Failed to run BehaviorTree for %s"), *InPawn->GetName());
		}
	}
}

void AAO_AIC_EnemyBase::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!HasAuthority() || !GetBlackboardComponent()) return;

	if (GetTeamAttitudeTowards(*Actor) != ETeamAttitude::Hostile) return;

	TSubclassOf<UAISense> SensedClass = UAIPerceptionSystem::GetSenseClassForStimulus(this, Stimulus);
	
	if (SensedClass == UAISense_Sight::StaticClass())
	{
		HandleSight(Actor, Stimulus);
	}
	else if (SensedClass == UAISense_Hearing::StaticClass())
	{
		HandleHearing(Actor, Stimulus);
	}
}

void AAO_AIC_EnemyBase::HandleSight(AActor* Actor, const FAIStimulus& Stimulus)
{
	if (Stimulus.WasSuccessfullySensed())
	{
		GetBlackboardComponent()->SetValueAsObject(TEXT("TargetPlayer"), Actor);
		GetBlackboardComponent()->SetValueAsBool(TEXT("bPlayerVisible"), true);
		PlayerMemoryMap.Add(Actor, Stimulus.StimulusLocation);
	}
	else
	{
		GetBlackboardComponent()->SetValueAsBool(TEXT("bPlayerVisible"), false);
		PlayerMemoryMap.Add(Actor, Stimulus.StimulusLocation);
	}
	
	GetBlackboardComponent()->SetValueAsVector(TEXT("LastKnownPlayerLocation"), Stimulus.StimulusLocation);
}

void AAO_AIC_EnemyBase::HandleHearing(AActor* Actor, const FAIStimulus& Stimulus)
{
	if (Stimulus.WasSuccessfullySensed())
	{
		GetBlackboardComponent()->SetValueAsVector(TEXT("LastKnownPlayerLocation"), Stimulus.StimulusLocation);
		PlayerMemoryMap.Add(Actor, Stimulus.StimulusLocation);
		
		AO_LOG(LogKSJ, Verbose, TEXT("Sensed Sound from: %s at %s"), *Actor->GetName(), *Stimulus.StimulusLocation.ToString());
	}
}

TArray<FVector> AAO_AIC_EnemyBase::GetAllLastKnownLocations() const
{
	TArray<FVector> Locations;
	PlayerMemoryMap.GenerateValueArray(Locations);
	return Locations;
}

bool AAO_AIC_EnemyBase::GetLastKnownLocationOf(AActor* TargetActor, FVector& OutLocation) const
{
	if (const FVector* Loc = PlayerMemoryMap.Find(TargetActor))
	{
		OutLocation = *Loc;
		return true;
	}
	return false;
}