#include "Train/AO_TrainDoor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

AAO_TrainDoor::AAO_TrainDoor(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;
    
    if (MeshComponent)
    {
        MeshComponent->SetVisibility(false);
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    
    DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
    RootComponent = DoorMesh; 
    
    DoorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    DoorMesh->SetCollisionResponseToAllChannels(ECR_Block);

    bIsToggleable = true;
    InteractionTitle = FText::FromString(TEXT("Door"));
    InteractionContent = FText::FromString(TEXT("Open/Close"));
}

void AAO_TrainDoor::BeginPlay()
{
    Super::BeginPlay();
    
    ClosedLocation = GetActorLocation();
    
    OpenedLocation = ClosedLocation + GetActorQuat().RotateVector(SlideOffset);

    bDoorOpen = false;
}

void AAO_TrainDoor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    FVector TargetLoc = bDoorOpen ? OpenedLocation : ClosedLocation;
    FVector CurrentLoc = GetActorLocation();
    
    if (!CurrentLoc.Equals(TargetLoc, 0.1f))
    {
        FVector NewLoc = FMath::VInterpTo(CurrentLoc, TargetLoc, DeltaTime, SlideSpeed);
        SetActorLocation(NewLoc);
    }
}

bool AAO_TrainDoor::CanInteraction(const FAO_InteractionQuery& InteractionQuery) const
{
    return Super::CanInteraction(InteractionQuery);
}

void AAO_TrainDoor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AAO_TrainDoor, bDoorOpen);
}

void AAO_TrainDoor::OnInteractionSuccess_BP_Implementation(AActor* Interactor)
{
    Super::OnInteractionSuccess_BP_Implementation(Interactor);

    if (!HasAuthority()) return;
    
    bDoorOpen = !bDoorOpen;
    
    // 서버에서도 소리 재생
    PlayDoorSound();
}

void AAO_TrainDoor::OnRep_DoorState()
{
    // 클라이언트에서 상태 변동 시 소리 재생
    PlayDoorSound();
}

void AAO_TrainDoor::PlayDoorSound()
{
    USoundBase* SoundToPlay = bDoorOpen ? DoorOpenSound : DoorCloseSound;
    if (SoundToPlay)
    {
        UGameplayStatics::PlaySoundAtLocation(this, SoundToPlay, GetActorLocation());
    }
}