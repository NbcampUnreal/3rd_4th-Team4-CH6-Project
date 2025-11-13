// AO_AIC_Crab.cpp

#include "AO/Public/AI/Controller/AO_AIC_Crab.h"
#include "Perception/AIPerceptionSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

AAO_AIC_Crab::AAO_AIC_Crab()
{
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComp"));

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 1500.0f;               // 감지 거리
	SightConfig->LoseSightRadius = 1800.0f;           // 시야 상실 거리
	SightConfig->PeripheralVisionAngleDegrees = 80.0f; // 시야각
	SightConfig->SetMaxAge(2.0f);                      // 감지 지속 시간
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

	AIPerceptionComp->ConfigureSense(*SightConfig);
	AIPerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());
	AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &AAO_AIC_Crab::OnTargetPerceptionUpdated);

	bStartAILogicOnPossess = true;
}

void AAO_AIC_Crab::BeginPlay()
{
	Super::BeginPlay();

	if (AIPerceptionComp)
	{
		UAIPerceptionSystem::RegisterPerceptionStimuliSource(this, UAISense_Sight::StaticClass(), this);
	}

	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		if (BB->GetValueAsObject(TEXT("PlayerActor")) == nullptr)
		{
			if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0))
			{
				BB->SetValueAsObject(TEXT("PlayerActor"), PlayerPawn);
			}
		}
	}
}

void AAO_AIC_Crab::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (BehaviorTreeAsset)
	{
		RunBehaviorTree(BehaviorTreeAsset);
	}
}

void AAO_AIC_Crab::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!GetBlackboardComponent())
	{
		return;
	}

	if (Stimulus.WasSuccessfullySensed())
	{
		GetBlackboardComponent()->SetValueAsObject(TEXT("PlayerActor"), Actor);
		GetBlackboardComponent()->SetValueAsBool(TEXT("bPlayerVisible"), true);
		GetBlackboardComponent()->SetValueAsVector(TEXT("LastKnownPlayerLocation"), Actor->GetActorLocation());
	}
	else
	{
		GetBlackboardComponent()->SetValueAsBool(TEXT("bPlayerVisible"), false);
	}
}

