// HSJ : AO_PinballTrigger.cpp
#include "Puzzle/Actor/Pinball/AO_PinballTrigger.h"
#include "Puzzle/Checker/AO_PuzzleConditionChecker.h"
#include "Components/BoxComponent.h"
#include "AO_Log.h"
#include "Puzzle/Actor/Pinball/AO_PinballBall.h"

AAO_PinballTrigger::AAO_PinballTrigger()
{
	bReplicates = true;

	TObjectPtr<USceneComponent> Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(RootComponent);
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
	TriggerBox->SetGenerateOverlapEvents(true);
	TriggerBox->SetBoxExtent(FVector(50, 50, 50));
}

void AAO_PinballTrigger::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AAO_PinballTrigger::OnTriggerOverlap);
	}
}

void AAO_PinballTrigger::OnTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;
	if (!Cast<AAO_PinballBall>(OtherActor)) return;

	if (TriggerMode == EPinballTriggerMode::Success)
	{
		AO_LOG_NET(LogHSJ, Warning, TEXT("Success!"));
        
		if (LinkedChecker && EventTag.IsValid())
		{
			LinkedChecker->OnPuzzleEvent(EventTag, this);
		}
	}
	else
	{
		if (PinballBall)
		{
			PinballBall->ResetToStart();
		}
        
		if (LinkedChecker && EventTag.IsValid())
		{
			LinkedChecker->OnPuzzleEvent(EventTag, this);
		}
	}
}