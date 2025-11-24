// HSJ : AO_OverwatchInspectionPuzzle.cpp
#include "Puzzle/Element/AO_OverwatchInspectionPuzzle.h"
#include "Interaction/Component/AO_InspectableComponent.h"
#include "Puzzle/Checker/AO_PuzzleConditionChecker.h"
#include "Net/UnrealNetwork.h"
#include "AO_Log.h"

AAO_OverwatchInspectionPuzzle::AAO_OverwatchInspectionPuzzle(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // 기본값은 위에서 내려다보는 카메라
    if (InspectableComponent)
    {
        InspectableComponent->CameraRelativeLocation = FVector(0.f, 0.f, 500.f);
        InspectableComponent->CameraRelativeRotation = FRotator(-90.f, 0.f, 0.f);
    }
}

void AAO_OverwatchInspectionPuzzle::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        ReplicatedExternalMappings = ExternalMeshMappings;
    }
}

void AAO_OverwatchInspectionPuzzle::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(AAO_OverwatchInspectionPuzzle, ReplicatedExternalMappings);
}

FAO_InspectionCameraSettings AAO_OverwatchInspectionPuzzle::GetInspectionCameraSettings() const
{
    FAO_InspectionCameraSettings Settings;
    
    Settings.CameraMode = CameraMode;
    Settings.MovementSpeed = CameraMovementSpeed;
    Settings.MovementBoundsExtent = MovementBoundsExtent;
    Settings.MovementType = bEnableCameraMovement ? EInspectionMovementType::Planar : EInspectionMovementType::None;

    if (CameraMode == EInspectionCameraMode::WorldAbsolute)
    {
        Settings.CameraLocation = AbsoluteCameraLocation;
        Settings.CameraRotation = AbsoluteCameraRotation;
    }
    else
    {
        // 상대좌표 모드
        if (InspectableComponent)
        {
            FTransform CameraTransform = InspectableComponent->GetInspectionCameraTransform();
            Settings.CameraLocation = CameraTransform.GetLocation();
            Settings.CameraRotation = CameraTransform.Rotator();
        }
    }

    return Settings;
}

bool AAO_OverwatchInspectionPuzzle::IsValidClickTarget(AActor* HitActor, UPrimitiveComponent* Component) const
{
    if (!HitActor)
    {
        return false;
    }

    for (const FAO_ExternalMeshMapping& Mapping : ExternalMeshMappings)
    {
        if (Mapping.TargetActor == HitActor)
        {
            return true;
        }
    }

    return false;
}

void AAO_OverwatchInspectionPuzzle::OnInspectionMeshClicked(UPrimitiveComponent* ClickedComponent)
{
    if (!ClickedComponent || !HasAuthority())
    {
        return;
    }

    AActor* HitActor = ClickedComponent->GetOwner();
    if (!HitActor)
    {
        return;
    }

    FGameplayTag FoundTag;

    // 자신의 내부 메시인지 확인
    if (HitActor == this)
    {
        FoundTag = FindTagForComponent(ClickedComponent);
    }
    // 외부 연결된 액터인지 확인
    else
    {
        FoundTag = FindTagForExternalComponent(HitActor, ClickedComponent);
    }

    if (!FoundTag.IsValid())
    {
        AO_LOG(LogHSJ, Warning, TEXT("No tag found for component: %s on actor: %s"), 
            *ClickedComponent->GetName(), *HitActor->GetName());
        return;
    }

    BroadcastTag(FoundTag);
}

FGameplayTag AAO_OverwatchInspectionPuzzle::FindTagForExternalComponent(AActor* HitActor, UPrimitiveComponent* Component) const
{
    if (!HitActor || !Component)
    {
        return FGameplayTag();
    }

    FName ComponentName = Component->GetFName();

    for (const FAO_ExternalMeshMapping& Mapping : ExternalMeshMappings)
    {
        if (Mapping.TargetActor == HitActor && Mapping.ComponentName == ComponentName)
        {
            return Mapping.ActivatedTag;
        }
    }

    return FGameplayTag();
}