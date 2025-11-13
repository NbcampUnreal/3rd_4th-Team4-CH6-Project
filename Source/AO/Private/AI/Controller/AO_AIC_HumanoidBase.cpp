// AO_AIC_HumanoidBase.cpp

#include "AI/Controller/AO_AIC_HumanoidBase.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "AI/Character/AO_Enemy_HumanoidBase.h"

AAO_AIC_HumanoidBase::AAO_AIC_HumanoidBase()
{
	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

	PerceptionComponent->ConfigureSense(*SightConfig);
	PerceptionComponent->SetDominantSense(UAISenseConfig_Sight::StaticClass());
	
	PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(
		this, &AAO_AIC_HumanoidBase::OnTargetPerceptionUpdated
	);
}

void AAO_AIC_HumanoidBase::BeginPlay()
{
	Super::BeginPlay();

	ControlledHumanoid = Cast<AAO_Enemy_HumanoidBase>(GetPawn());
	if(!ControlledHumanoid)
	{
		return;
	}
	
	if(ControlledHumanoid->BehaviorTreeAsset)
	{
		RunBehaviorTree(ControlledHumanoid->BehaviorTreeAsset);
	}

	BlackboardComp = GetBlackboardComponent();
}

void AAO_AIC_HumanoidBase::OnTargetPerceptionUpdated(
	AActor* Actor, FAIStimulus Stimulus
)
{
	if(!ControlledHumanoid || !BlackboardComp)
	{
		return;
	}
	
	if(Stimulus.WasSuccessfullySensed())
	{
		if(ControlledHumanoid->HasAuthority())
		{
			ControlledHumanoid->SetCurrentTarget(Actor);
			BlackboardComp->SetValueAsObject(Key_TargetActor, Actor);
			BlackboardComp->SetValueAsBool(Key_HasLineOfSight, true);
			
		}
	}
	else
	{
		if(ControlledHumanoid->HasAuthority())
		{
			FVector LastSeen = Stimulus.StimulusLocation;

			ControlledHumanoid->SetLastSeenLocation(LastSeen);
			BlackboardComp->SetValueAsVector(Key_LastSeenLocation, LastSeen);
			BlackboardComp->SetValueAsBool(Key_HasLineOfSight, false);
			BlackboardComp->SetValueAsBool(Key_IsSearching, true);
		}
	}
}
