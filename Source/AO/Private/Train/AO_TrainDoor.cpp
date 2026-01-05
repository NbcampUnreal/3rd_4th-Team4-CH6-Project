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
    InteractionTitle = FText::FromString(TEXT("열차 문"));
    InteractionContent = FText::FromString(TEXT("열기/닫기"));
}

void AAO_TrainDoor::BeginPlay()
{
    Super::BeginPlay();
    
    ClosedRotation = GetActorRotation();
    OpenedRotation = ClosedRotation + FRotator(0.0f, TargetRotationYaw, 0.0f);

    bDoorOpen = false;
}

void AAO_TrainDoor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    FRotator TargetRot = bDoorOpen ? OpenedRotation : ClosedRotation;
    
    FRotator CurrentRot = GetActorRotation();
    
    if (!CurrentRot.Equals(TargetRot, 0.01f))
    {
        FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, RotationSpeed);
        SetActorRotation(NewRot);
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
    
    PlayDoorSound();
}

void AAO_TrainDoor::OnRep_DoorState()
{
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