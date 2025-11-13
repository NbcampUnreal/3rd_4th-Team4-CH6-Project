// AO_AIC_HumanoidBase.cpp

#include "AI/Controller/AO_AIC_HumanoidBase.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "AI/Character/AO_Enemy_HumanoidBase.h"

AAO_AIC_HumanoidBase::AAO_AIC_HumanoidBase()
{
	// Perception Component
	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

	PerceptionComponent->ConfigureSense(*SightConfig);
	PerceptionComponent->SetDominantSense(UAISenseConfig_Sight::StaticClass());

	// Perception Event
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

	// Behavior Tree 실행
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

	// 플레이어 감지됨
	if(Stimulus.WasSuccessfullySensed())
	{
		// 서버에서만 타깃 설정
		if(ControlledHumanoid->HasAuthority())
		{
			ControlledHumanoid->SetCurrentTarget(Actor);
			BlackboardComp->SetValueAsObject(Key_TargetActor, Actor);
			BlackboardComp->SetValueAsBool(Key_HasLineOfSight, true);

			// Chase 상태는 BT에서 처리됨
		}
	}
	else
	{
		// 감지 잃어버림 → 마지막 위치 저장
		if(ControlledHumanoid->HasAuthority())
		{
			FVector LastSeen = Stimulus.StimulusLocation;

			ControlledHumanoid->SetLastSeenLocation(LastSeen);
			BlackboardComp->SetValueAsVector(Key_LastSeenLocation, LastSeen);
			BlackboardComp->SetValueAsBool(Key_HasLineOfSight, false);
		}
	}
}
