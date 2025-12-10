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
	Settings.bHideCharacter = bHideCharacter;

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
	if (!HitActor || !Component)
	{
		return false;
	}

	FName ComponentName = Component->GetFName();

	for (const FAO_ExternalMeshMapping& Mapping : ExternalMeshMappings)
	{
		if (Mapping.TargetActor == HitActor && Mapping.ComponentName == ComponentName)
		{
			return true;
		}
	}

    return false;
}

void AAO_OverwatchInspectionPuzzle::ActiveAllLinkedElements()
{
    if (!HasAuthority())
    {
        AO_LOG(LogHSJ, Warning, TEXT("[ActiveAll] Not authority, ignoring"));
        return;
    }

    if (ExternalMeshMappings.Num() == 0)
    {
        AO_LOG(LogHSJ, Warning, TEXT("[ActiveAll] No external mesh mappings"));
        return;
    }

    int32 ActivatedCount = 0;

    for (int32 i = 0; i < ExternalMeshMappings.Num(); ++i)
    {
        const FAO_ExternalMeshMapping& Mapping = ExternalMeshMappings[i];
        
        if (!Mapping.TargetActor)
        {
            continue;
        }

        if (!Mapping.TargetActor->GetClass()->ImplementsInterface(UAO_Interface_Inspectable::StaticClass()))
        {
            continue;
        }

        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Mapping.TargetActor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);

        bool bFoundComponent = false;
        for (TObjectPtr<UPrimitiveComponent> Comp : PrimitiveComponents)
        {
            if (Comp && Comp->GetFName() == Mapping.ComponentName)
            {
                bFoundComponent = true;
                
                IAO_Interface_Inspectable* InspectableActor = Cast<IAO_Interface_Inspectable>(Mapping.TargetActor);
                if (InspectableActor)
                {
                    InspectableActor->OnInspectionMeshClicked(Comp);
                    ActivatedCount++;
                }
                break;
            }
        }
    }
}

void AAO_OverwatchInspectionPuzzle::HighlightAllExternalMeshes()
{
	// 스페이스바 모드가 아니면 하이라이트 안 함
	if (!bUseSpacebar)
	{
		return;
	}

	// 기존 하이라이트 클리어 (혹시 남아있을 수 있음)
	ClearAllExternalHighlights();

	// 모든 외부 메시 하이라이트
	for (const FAO_ExternalMeshMapping& Mapping : ExternalMeshMappings)
	{
		if (!Mapping.TargetActor)
		{
			continue;
		}

		// 해당 액터에서 컴포넌트 찾기
		TArray<UPrimitiveComponent*> PrimitiveComponents;
		Mapping.TargetActor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);

		for (TObjectPtr<UPrimitiveComponent> Comp : PrimitiveComponents)
		{
			if (Comp && Comp->GetFName() == Mapping.ComponentName)
			{
				// 메시 컴포넌트면 하이라이트 적용
				if (TObjectPtr<UMeshComponent> MeshComp = Cast<UMeshComponent>(Comp))
				{
					MeshComp->SetRenderCustomDepth(true);
					MeshComp->SetCustomDepthStencilValue(250);
					HighlightedComponents.Add(Comp);
				}
				break;
			}
		}
	}
}

void AAO_OverwatchInspectionPuzzle::ClearAllExternalHighlights()
{
	// 모든 하이라이트 해제
	for (TWeakObjectPtr<UPrimitiveComponent>& WeakComp : HighlightedComponents)
	{
		if (TObjectPtr<UPrimitiveComponent> Comp = WeakComp.Get())
		{
			Comp->SetRenderCustomDepth(false);
		}
	}

	HighlightedComponents.Empty();
}

void AAO_OverwatchInspectionPuzzle::OnInspectionMeshClicked(UPrimitiveComponent* ClickedComponent)
{
    if (!ClickedComponent || !HasAuthority())
    {
        return;
    }

	if (bUseSpacebar)
	{
		return;
	}

    TObjectPtr<AActor> HitActor = ClickedComponent->GetOwner();
    if (!HitActor)
    {
        return;
    }

	// 외부 액터이면서 IAO_Interface_Inspectable을 구현하는지 체크(자기 자신은 제외, 사용예시:회전 퍼즐 액터)
	if (HitActor != this && HitActor->GetClass()->ImplementsInterface(UAO_Interface_Inspectable::StaticClass()))
	{
		IAO_Interface_Inspectable* InspectableActor = Cast<IAO_Interface_Inspectable>(HitActor);
		if (InspectableActor)
		{
			// 회전 퍼즐 등 자체적으로 클릭 처리하는 액터
			InspectableActor->OnInspectionMeshClicked(ClickedComponent);
			return;
		}
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