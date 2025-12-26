// HSJ : AO_CableCarReactionActor.cpp
#include "Puzzle/Actor/AO_CableCarReactionActor.h"
#include "Components/SplineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

AAO_CableCarReactionActor::AAO_CableCarReactionActor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    ReactionMode = EPuzzleReactionMode::Toggle;

    SplinePath = CreateDefaultSubobject<USplineComponent>(TEXT("SplinePath"));
    SplinePath->SetupAttachment(RootComponent);
    
    SplinePath->ClearSplinePoints();
    SplinePath->AddSplinePoint(FVector(0, 0, 0), ESplineCoordinateSpace::Local);
    SplinePath->AddSplinePoint(FVector(0, 0, 1000), ESplineCoordinateSpace::Local);
    
    for (int32 i = 0; i < SplinePath->GetNumberOfSplinePoints(); ++i)
    {
        SplinePath->SetSplinePointType(i, ESplinePointType::Curve);
    }
    
    SplinePath->UpdateSpline();
}

void AAO_CableCarReactionActor::BeginPlay()
{
    Super::BeginPlay();

    if (MeshComponent)
    {
        MeshComponent->SetSimulatePhysics(false);
        MeshComponent->SetEnableGravity(false);
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
    }

    if (SplinePath)
    {
        int32 NumPoints = SplinePath->GetNumberOfSplinePoints();
        SplineWorldPoints.Reserve(NumPoints);
        
        for (int32 i = 0; i < NumPoints; ++i)
        {
            FVector WorldPoint = SplinePath->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);
            SplineWorldPoints.Add(WorldPoint);
        }
    }

    CurrentAlpha = 0.0f;
    
    if (SplineWorldPoints.Num() >= 2)
    {
        SetActorLocation(SplineWorldPoints[0]);
    }
}

void AAO_CableCarReactionActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(AAO_CableCarReactionActor, MovementCommandCounter);
    DOREPLIFETIME(AAO_CableCarReactionActor, bTargetIsForward);
}

void AAO_CableCarReactionActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bIsMoving)
    {
    	return;
    }

    float DistanceToTarget = FMath::Abs(TargetAlpha - CurrentAlpha);
    float EasedSpeed = MovementSpeed * DeltaTime * ApplyEaseInOutCurve(DistanceToTarget);
    
    if (bIsMovingForward)
    {
        CurrentAlpha = FMath::Min(CurrentAlpha + EasedSpeed, TargetAlpha);
    }
    else
    {
        CurrentAlpha = FMath::Max(CurrentAlpha - EasedSpeed, TargetAlpha);
    }

    CurrentAlpha = FMath::Clamp(CurrentAlpha, 0.0f, 1.0f);

    if (SplineWorldPoints.Num() >= 2)
    {
        FVector StartPoint = SplineWorldPoints[0];
        FVector EndPoint = SplineWorldPoints[SplineWorldPoints.Num() - 1];
        FVector TargetLocation = FMath::Lerp(StartPoint, EndPoint, CurrentAlpha);
        
        FVector Delta = TargetLocation - GetActorLocation();
        
        if (!Delta.IsNearlyZero(0.01f))
        {
            AddActorWorldOffset(Delta, true);
        }
    }

    if (FMath::IsNearlyEqual(CurrentAlpha, TargetAlpha, 0.001f))
    {
        CurrentAlpha = TargetAlpha;
        StopMovement();
    }
}

void AAO_CableCarReactionActor::ActivateReaction()
{
    if (!HasAuthority())
    {
    	return;
    }

    if (bIsMoving)
    {
        bTargetIsForward = !bIsMovingForward;
        bIsMovingForward = bTargetIsForward;
        TargetAlpha = bIsMovingForward ? 1.0f : 0.0f;
        
        if (MovementStartSound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, MovementStartSound, GetActorLocation());
        }
        
        MovementCommandCounter++;
    }
    else
    {
        bTargetIsForward = true;
        StartMovement(true);
        MovementCommandCounter++;
    }
    
    bIsActivated = true;
}

void AAO_CableCarReactionActor::DeactivateReaction()
{
    if (!HasAuthority())
    {
    	return;
    }

    if (bIsMoving)
    {
        bTargetIsForward = !bIsMovingForward;
        bIsMovingForward = bTargetIsForward;
        TargetAlpha = bIsMovingForward ? 1.0f : 0.0f;
        
        if (MovementStartSound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, MovementStartSound, GetActorLocation());
        }
        
        MovementCommandCounter++;
    }
    else
    {
        bTargetIsForward = false;
        StartMovement(false);
        MovementCommandCounter++;
    }
    
    bIsActivated = false;
}

void AAO_CableCarReactionActor::StartMovement(bool bForward)
{
    bIsMovingForward = bForward;
    TargetAlpha = bForward ? 1.0f : 0.0f;
    bIsMoving = true;

    if (MovementStartSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, MovementStartSound, GetActorLocation());
    }
}

void AAO_CableCarReactionActor::StopMovement()
{
    bIsMoving = false;
    
    if (MovementStopSound && HasAuthority())
    {
        UGameplayStatics::PlaySoundAtLocation(this, MovementStopSound, GetActorLocation());
    }
}

float AAO_CableCarReactionActor::ApplyEaseInOutCurve(float DistanceRemaining) const
{
    float T = FMath::Clamp(DistanceRemaining, 0.0f, 1.0f);
    float EaseValue = T * T * (3.0f - 2.0f * T);
    float PoweredEase = FMath::Pow(EaseValue, 1.0f / EaseStrength);
    return FMath::Lerp(0.1f, 1.0f, PoweredEase);
}

void AAO_CableCarReactionActor::OnRep_MovementCommand()
{
    if (!HasAuthority() && MovementCommandCounter != LastProcessedCommand)
    {
        LastProcessedCommand = MovementCommandCounter;
        
        if (bIsMoving)
        {
            bIsMovingForward = bTargetIsForward;
            TargetAlpha = bIsMovingForward ? 1.0f : 0.0f;
            
            if (MovementStartSound)
            {
                UGameplayStatics::PlaySoundAtLocation(this, MovementStartSound, GetActorLocation());
            }
        }
        else
        {
            bIsMovingForward = bTargetIsForward;
            TargetAlpha = bIsMovingForward ? 1.0f : 0.0f;
            bIsMoving = true;
            
            if (MovementStartSound)
            {
                UGameplayStatics::PlaySoundAtLocation(this, MovementStartSound, GetActorLocation());
            }
        }
    }
}