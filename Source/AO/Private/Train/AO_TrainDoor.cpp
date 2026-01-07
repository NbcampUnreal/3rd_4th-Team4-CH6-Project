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
    
    // 시작 위치 저장
    ClosedLocation = GetActorLocation();
    
    // 로컬 좌표 기준 Offset을 월드 좌표로 변환하여 목표 위치 계산
    OpenedLocation = ClosedLocation + GetActorQuat().RotateVector(SlideOffset);

    bDoorOpen = false;
}

void AAO_TrainDoor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // 목표 위치 설정
    FVector TargetLoc = bDoorOpen ? OpenedLocation : ClosedLocation;
    FVector CurrentLoc = GetActorLocation();
    
    // 현재 위치가 목표 위치와 차이가 있다면 이동
    if (!CurrentLoc.Equals(TargetLoc, 0.1f))
    {
        // VInterpTo를 사용하여 부드러운 슬라이딩 구현
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