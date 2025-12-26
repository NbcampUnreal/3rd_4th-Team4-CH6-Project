// HSJ : AO_ElevatorReactionActor.cpp
#include "Puzzle/Actor/AO_ElevatorReactionActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

AAO_ElevatorReactionActor::AAO_ElevatorReactionActor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    LeftDoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftDoorMesh"));
    LeftDoorMesh->SetupAttachment(RootComponent);
    LeftDoorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    LeftDoorMesh->SetCollisionResponseToAllChannels(ECR_Block);

    RightDoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightDoorMesh"));
    RightDoorMesh->SetupAttachment(RootComponent);
    RightDoorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    RightDoorMesh->SetCollisionResponseToAllChannels(ECR_Block);
}

void AAO_ElevatorReactionActor::BeginPlay()
{
    Super::BeginPlay();

    if (MeshComponent)
    {
        InitialLocation = MeshComponent->GetRelativeLocation();
        InitialRotation = MeshComponent->GetRelativeRotation();
        
        MeshComponent->SetSimulatePhysics(false);
        MeshComponent->SetEnableGravity(false);
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
    }

    InitialActorLocation = GetActorLocation();
    InitialActorRotation = GetActorRotation();

    if (LeftDoorMesh)
    {
        LeftDoorInitialLocation = LeftDoorMesh->GetRelativeLocation();
    }
    if (RightDoorMesh)
    {
        RightDoorInitialLocation = RightDoorMesh->GetRelativeLocation();
    }

    CurrentProgress = 0.0f;
    TargetProgressValue = 0.0f;
    CurrentDoorProgress = 0.0f;
    TargetDoorProgress = 0.0f;
}

void AAO_ElevatorReactionActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    TObjectPtr<UWorld> World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(TransformTimerHandle);
        World->GetTimerManager().ClearTimer(DoorTimerHandle);
    }
    
    Super::EndPlay(EndPlayReason);
}

void AAO_ElevatorReactionActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(AAO_ElevatorReactionActor, SequenceCounter);
    DOREPLIFETIME(AAO_ElevatorReactionActor, ReplicatedStepQueue);
}

void AAO_ElevatorReactionActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsMoving)
    {
        UpdateElevatorMovement(DeltaTime);
    }

    if (bIsDoorMoving)
    {
        UpdateDoorMovement(DeltaTime);
    }
}

void AAO_ElevatorReactionActor::CallFromGround()
{
    if (!HasAuthority())
    {
    	return;
    }
    if (bIsProcessing)
    {
    	return;
    }

    BuildSequenceForGroundButton();
    bIsProcessing = true;
    SequenceCounter++;
    ExecuteNextStep();
}

void AAO_ElevatorReactionActor::CallFromBasement()
{
	if (!HasAuthority())
	{
		return;
	}
	if (bIsProcessing)
	{
		return;
	}

    BuildSequenceForBasementButton();
    bIsProcessing = true;
    SequenceCounter++;
    ExecuteNextStep();
}

void AAO_ElevatorReactionActor::CallFromInterior()
{
	if (!HasAuthority())
	{
		return;
	}
	if (bIsProcessing)
	{
		return;
	}

    BuildSequenceForInteriorButton();
    bIsProcessing = true;
    SequenceCounter++;
    ExecuteNextStep();
}

void AAO_ElevatorReactionActor::BuildSequenceForGroundButton()
{
    StepQueue.Empty();
    CurrentStepIndex = 0;

    if (CurrentProgress < 0.5f)  // 엘레베이터 위치가 지상인 경우
    {
        StepQueue.Add(EElevatorStep::OpenDoor);
        StepQueue.Add(EElevatorStep::WaitDoor);
        StepQueue.Add(EElevatorStep::CloseDoor);
        StepQueue.Add(EElevatorStep::MoveToBasement);
        StepQueue.Add(EElevatorStep::OpenDoor);
        StepQueue.Add(EElevatorStep::WaitDoor);
        StepQueue.Add(EElevatorStep::CloseDoor);
        StepQueue.Add(EElevatorStep::Complete);
    }
    else  // 엘레베이터 위치가 지하인 경우
    {
        StepQueue.Add(EElevatorStep::MoveToGround);
        StepQueue.Add(EElevatorStep::OpenDoor);
        StepQueue.Add(EElevatorStep::WaitDoor);
        StepQueue.Add(EElevatorStep::CloseDoor);
        StepQueue.Add(EElevatorStep::MoveToBasement);
        StepQueue.Add(EElevatorStep::OpenDoor);
        StepQueue.Add(EElevatorStep::WaitDoor);
        StepQueue.Add(EElevatorStep::CloseDoor);
        StepQueue.Add(EElevatorStep::Complete);
    }

    ReplicatedStepQueue = StepQueue;
}

void AAO_ElevatorReactionActor::BuildSequenceForBasementButton()
{
    StepQueue.Empty();
    CurrentStepIndex = 0;

    if (CurrentProgress >= 0.5f)  // 엘레베이터 위치가 지하인 경우
    {
        StepQueue.Add(EElevatorStep::OpenDoor);
        StepQueue.Add(EElevatorStep::WaitDoor);
        StepQueue.Add(EElevatorStep::CloseDoor);
        StepQueue.Add(EElevatorStep::MoveToGround);
        StepQueue.Add(EElevatorStep::OpenDoor);
        StepQueue.Add(EElevatorStep::WaitDoor);
        StepQueue.Add(EElevatorStep::CloseDoor);
        StepQueue.Add(EElevatorStep::Complete);
    }
    else  // 엘레베이터 위치가 지상인 경우
    {
        StepQueue.Add(EElevatorStep::MoveToBasement);
        StepQueue.Add(EElevatorStep::OpenDoor);
        StepQueue.Add(EElevatorStep::WaitDoor);
        StepQueue.Add(EElevatorStep::CloseDoor);
        StepQueue.Add(EElevatorStep::MoveToGround);
        StepQueue.Add(EElevatorStep::OpenDoor);
        StepQueue.Add(EElevatorStep::WaitDoor);
        StepQueue.Add(EElevatorStep::CloseDoor);
        StepQueue.Add(EElevatorStep::Complete);
    }

    ReplicatedStepQueue = StepQueue;
}

void AAO_ElevatorReactionActor::BuildSequenceForInteriorButton()
{
    StepQueue.Empty();
    CurrentStepIndex = 0;

    if (CurrentProgress < 0.5f)  // 지상에서 지하로 가는 경우
    {
        StepQueue.Add(EElevatorStep::MoveToBasement);
        StepQueue.Add(EElevatorStep::OpenDoor);
        StepQueue.Add(EElevatorStep::WaitDoor);
        StepQueue.Add(EElevatorStep::CloseDoor);
        StepQueue.Add(EElevatorStep::Complete);
    }
    else  // 지하에서 지상으로 가는 경우
    {
        StepQueue.Add(EElevatorStep::MoveToGround);
        StepQueue.Add(EElevatorStep::OpenDoor);
        StepQueue.Add(EElevatorStep::WaitDoor);
        StepQueue.Add(EElevatorStep::CloseDoor);
        StepQueue.Add(EElevatorStep::Complete);
    }

    ReplicatedStepQueue = StepQueue;
}

void AAO_ElevatorReactionActor::ExecuteNextStep()
{
    if (CurrentStepIndex >= StepQueue.Num())
    {
        bIsProcessing = false;
        return;
    }

    EElevatorStep Step = StepQueue[CurrentStepIndex];
    ExecuteStep(Step);
}

void AAO_ElevatorReactionActor::ExecuteStep(EElevatorStep Step)
{
    switch (Step)
    {
    case EElevatorStep::OpenDoor:
        OpenDoor();
        break;

    case EElevatorStep::WaitDoor:
        StartDoorTimer();
        break;

    case EElevatorStep::CloseDoor:
        CloseDoor();
        break;

    case EElevatorStep::MoveToGround:
        MoveToGround();
        break;

    case EElevatorStep::MoveToBasement:
        MoveToBasement();
        break;

    case EElevatorStep::Complete:
        bIsProcessing = false;
        break;
    }
}

void AAO_ElevatorReactionActor::OnStepComplete()
{
    CurrentStepIndex++;
    ExecuteNextStep();
}

void AAO_ElevatorReactionActor::OpenDoor()
{
    TargetDoorProgress = 1.0f;
    bIsDoorMoving = true;
    
    if (DoorOpenSound && HasAuthority())
    {
        UGameplayStatics::PlaySoundAtLocation(this, DoorOpenSound, GetActorLocation());
    }
}

void AAO_ElevatorReactionActor::CloseDoor()
{
    TargetDoorProgress = 0.0f;
    bIsDoorMoving = true;
    
    if (DoorCloseSound && HasAuthority())
    {
        UGameplayStatics::PlaySoundAtLocation(this, DoorCloseSound, GetActorLocation());
    }
}

void AAO_ElevatorReactionActor::UpdateDoorMovement(float DeltaTime)
{
    float PreviousDoorProgress = CurrentDoorProgress;
    CurrentDoorProgress = FMath::FInterpConstantTo(CurrentDoorProgress, TargetDoorProgress, DeltaTime, DoorSpeed);

    if (LeftDoorMesh)
    {
        FVector LeftOffset = FVector(-DoorOpenDistance * CurrentDoorProgress, 0, 0);
        LeftDoorMesh->SetRelativeLocation(LeftDoorInitialLocation + LeftOffset);
    }

    if (RightDoorMesh)
    {
        FVector RightOffset = FVector(DoorOpenDistance * CurrentDoorProgress, 0, 0);
        RightDoorMesh->SetRelativeLocation(RightDoorInitialLocation + RightOffset);
    }

    if (FMath::IsNearlyEqual(CurrentDoorProgress, TargetDoorProgress, 0.001f))
    {
        CurrentDoorProgress = TargetDoorProgress;
        bIsDoorMoving = false;
        OnStepComplete();
    }
}

void AAO_ElevatorReactionActor::StartDoorTimer()
{
    TObjectPtr<UWorld> World = GetWorld();
    if (World)
    {
        TWeakObjectPtr<AAO_ElevatorReactionActor> WeakThis(this);
        World->GetTimerManager().SetTimer(
            DoorTimerHandle,
            FTimerDelegate::CreateWeakLambda(this, [WeakThis]()
            {
                if (TObjectPtr<AAO_ElevatorReactionActor> StrongThis = WeakThis.Get())
                {
                    StrongThis->OnDoorTimerComplete();
                }
            }),
            DoorOpenDuration,
            false
        );
    }
}

void AAO_ElevatorReactionActor::OnDoorTimerComplete()
{
    OnStepComplete();
}

void AAO_ElevatorReactionActor::MoveToGround()
{
    TargetProgressValue = 0.0f;
    bIsMoving = true;
    
    if (MovementStartSound && HasAuthority())
    {
        UGameplayStatics::PlaySoundAtLocation(this, MovementStartSound, GetActorLocation());
    }
}

void AAO_ElevatorReactionActor::MoveToBasement()
{
    TargetProgressValue = 1.0f;
    bIsMoving = true;
    
    if (MovementStartSound && HasAuthority())
    {
        UGameplayStatics::PlaySoundAtLocation(this, MovementStartSound, GetActorLocation());
    }
}

void AAO_ElevatorReactionActor::UpdateElevatorMovement(float DeltaTime)
{
    if (!MeshComponent)
    {
    	return;
    }

    float DistanceToTarget = FMath::Abs(TargetProgressValue - CurrentProgress);
    float EasedSpeed = ElevatorSpeed * DeltaTime * ApplyEaseInOutCurve(DistanceToTarget);
    
    if (TargetProgressValue > CurrentProgress)
    {
        CurrentProgress = FMath::Min(CurrentProgress + EasedSpeed, TargetProgressValue);
    }
    else
    {
        CurrentProgress = FMath::Max(CurrentProgress - EasedSpeed, TargetProgressValue);
    }

    CurrentProgress = FMath::Clamp(CurrentProgress, 0.0f, 1.0f);

    FVector StartOffset = FVector::ZeroVector;
    FVector EndOffset = bUseLocation ? (TargetRelativeLocation - InitialLocation) : FVector::ZeroVector;
    FVector CurrentOffset = FMath::Lerp(StartOffset, EndOffset, CurrentProgress);

    FVector DesiredWorldLocation = InitialActorLocation + CurrentOffset;

    FRotator DesiredWorldRotation = InitialActorRotation;
    if (bUseRotation)
    {
        FRotator RotationOffset = TargetRelativeRotation - InitialRotation;
        FRotator CurrentRotationOffset = FMath::Lerp(FRotator::ZeroRotator, RotationOffset, CurrentProgress);
        DesiredWorldRotation = InitialActorRotation + CurrentRotationOffset;
    }

    SetActorLocation(DesiredWorldLocation, false, nullptr, ETeleportType::TeleportPhysics);

    if (bUseRotation)
    {
        SetActorRotation(DesiredWorldRotation, ETeleportType::TeleportPhysics);
    }

    if (FMath::IsNearlyEqual(CurrentProgress, TargetProgressValue, 0.001f))
    {
        CurrentProgress = TargetProgressValue;
        
        FVector FinalOffset = FMath::Lerp(StartOffset, EndOffset, CurrentProgress);
        FVector FinalWorldLocation = InitialActorLocation + FinalOffset;
        SetActorLocation(FinalWorldLocation, false, nullptr, ETeleportType::TeleportPhysics);
        
        bIsMoving = false;
        
        if (MovementStopSound && HasAuthority())
        {
            UGameplayStatics::PlaySoundAtLocation(this, MovementStopSound, GetActorLocation());
        }
        
        OnStepComplete();
    }
}

float AAO_ElevatorReactionActor::ApplyEaseInOutCurve(float DistanceRemaining) const
{
    float T = FMath::Clamp(DistanceRemaining, 0.0f, 1.0f);
    float EaseValue = T * T * (3.0f - 2.0f * T);
    float PoweredEase = FMath::Pow(EaseValue, 1.0f / EaseStrength);
    
    return FMath::Lerp(0.1f, 1.0f, PoweredEase);
}

void AAO_ElevatorReactionActor::OnRep_ElevatorSequence()
{
    if (!HasAuthority() && SequenceCounter != LastProcessedSequence)
    {
        LastProcessedSequence = SequenceCounter;
        
        StepQueue = ReplicatedStepQueue;
        CurrentStepIndex = 0;
        bIsProcessing = true;
        
        ExecuteNextStep();
    }
}