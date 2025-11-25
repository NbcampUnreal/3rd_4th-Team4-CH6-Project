// HSJ : AO_PressurePlate.cpp
#include "Puzzle/Element/AO_PressurePlate.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"

AAO_PressurePlate::AAO_PressurePlate(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // 압력판은 OneTime 타입 기반 (한번 활성화 후 유지되는 게 아니라 Overlap으로 제어)
    ElementType = EPuzzleElementType::OneTime;

    OverlapTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapTrigger"));
    OverlapTrigger->SetupAttachment(RootComponent);
    OverlapTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    OverlapTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
    OverlapTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    OverlapTrigger->SetGenerateOverlapEvents(true);
}

void AAO_PressurePlate::BeginPlay()
{
    Super::BeginPlay();

    if (OverlapTrigger)
    {
        OverlapTrigger->SetBoxExtent(TriggerExtent);
        OverlapTrigger->OnComponentBeginOverlap.AddDynamic(this, &AAO_PressurePlate::OnOverlapBegin);
        OverlapTrigger->OnComponentEndOverlap.AddDynamic(this, &AAO_PressurePlate::OnOverlapEnd);
    }
}

bool AAO_PressurePlate::CanInteraction(const FAO_InteractionQuery& InteractionQuery) const
{
    // F키 상호작용 비활성화
    return false;
}

void AAO_PressurePlate::ResetToInitialState()
{
    Super::ResetToInitialState();
    OverlappingActors.Empty();
}

void AAO_PressurePlate::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!HasAuthority()) return;
    if (!bInteractionEnabled) return;

    APawn* Pawn = Cast<APawn>(OtherActor);
    if (!Pawn) return;

    if (OverlappingActors.Contains(OtherActor)) return;

    OverlappingActors.Add(OtherActor);

    // 첫 번째 액터가 들어오면 활성화
    if (OverlappingActors.Num() == 1)
    {
        bIsActivated = true;
        BroadcastPuzzleEvent(true);
        OnRep_IsActivated();
    }
}

void AAO_PressurePlate::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!HasAuthority()) return;

    OverlappingActors.Remove(OtherActor);

    // 모든 액터가 나가면 비활성화
    if (OverlappingActors.Num() == 0)
    {
        bIsActivated = false;
        BroadcastPuzzleEvent(false);
        OnRep_IsActivated();
    }
}