// HSJ : AO_InspectionPuzzle.cpp
#include "Puzzle/Element/AO_InspectionPuzzle.h"
#include "Interaction/Component/AO_InspectableComponent.h"
#include "Puzzle/Checker/AO_PuzzleConditionChecker.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "AO_Log.h"
#include "Interaction/Component/AO_InspectionComponent.h"

AAO_InspectionPuzzle::AAO_InspectionPuzzle(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bReplicates = true;

    RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
    RootComponent = RootComp;

    InspectableComponent = CreateDefaultSubobject<UAO_InspectableComponent>(TEXT("InspectableComponent"));
}

void AAO_InspectionPuzzle::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AAO_InspectionPuzzle, ReplicatedMappings);
	DOREPLIFETIME(AAO_InspectionPuzzle, bInteractionEnabled);
}

FAO_InspectionCameraSettings AAO_InspectionPuzzle::GetInspectionCameraSettings() const
{
	FAO_InspectionCameraSettings Settings;
    
	// 기본 카메라 설정
	if (InspectableComponent)
	{
		FTransform CameraTransform = InspectableComponent->GetInspectionCameraTransform();
		Settings.CameraMode = EInspectionCameraMode::RelativeToActor;
		Settings.CameraLocation = CameraTransform.GetLocation();
		Settings.CameraRotation = CameraTransform.Rotator();
	}
    
	Settings.MovementType = EInspectionMovementType::None;
	Settings.bHideCharacter = bHideCharacter;
    
	return Settings;
}

bool AAO_InspectionPuzzle::IsInternalComponentClickable(FName ComponentName) const
{
	for (const FAO_InspectionElementMapping& Mapping : ElementMappings)
	{
		if (Mapping.MeshComponentName == ComponentName)
		{
			return true;
		}
	}
	return false;
}

void AAO_InspectionPuzzle::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        ReplicatedMappings = ElementMappings;
    }
}

FAO_InteractionInfo AAO_InspectionPuzzle::GetInteractionInfo(const FAO_InteractionQuery& InteractionQuery) const
{
    FAO_InteractionInfo Info;
    Info.Title = InspectionTitle;
    Info.Content = InspectionContent;
    Info.Duration = InteractionDuration;
    Info.AbilityToGrant = AbilityToGrant;
    return Info;
}

bool AAO_InspectionPuzzle::CanInteraction(const FAO_InteractionQuery& InteractionQuery) const
{
    if (!InspectableComponent)
    {
        return false;
    }

	if (!bInteractionEnabled)
	{
		return false;
	}
    
	TObjectPtr<AActor> RequestingActor = InteractionQuery.RequestingAvatar.Get();
    if (!RequestingActor)
    {
        return false;
    }
    
    // 다른 플레이어가 검사 중이면 불가
    if (InspectableComponent->IsLockedByOtherPlayer(RequestingActor))
    {
        return false;
    }
    
    // 이미 자신이 검사 중이면 불가
	TObjectPtr<UAO_InspectionComponent> InspectionComp = RequestingActor->FindComponentByClass<UAO_InspectionComponent>();
    if (InspectionComp && InspectionComp->GetInspectedActor() == this)
    {
        return false;
    }
    
    return true;
}

void AAO_InspectionPuzzle::GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const
{
	// 액터의 모든 메시 컴포넌트 수집 (재귀)
	TArray<UMeshComponent*> AllMeshComponents;
	GetComponents<UMeshComponent>(AllMeshComponents, true);
    
	for (TObjectPtr<UMeshComponent> MeshComp : AllMeshComponents)
	{
		if (!MeshComp) continue;
        
		if (TObjectPtr<UStaticMeshComponent> StaticMesh = Cast<UStaticMeshComponent>(MeshComp))
		{
			if (StaticMesh->GetStaticMesh())
			{
				OutMeshComponents.Add(MeshComp);
			}
		}
	}
}

void AAO_InspectionPuzzle::OnInspectionMeshClicked(UPrimitiveComponent* ClickedComponent)
{
    if (!ClickedComponent || !HasAuthority())
    {
        return;
    }

    FGameplayTag FoundTag = FindTagForComponent(ClickedComponent);
    if (!FoundTag.IsValid())
    {
        AO_LOG(LogHSJ, Warning, TEXT("No tag found for component: %s"), *ClickedComponent->GetName());
        return;
    }

    AO_LOG(LogHSJ, Warning, TEXT("Mesh clicked: %s, Broadcasting tag: %s"), 
        *ClickedComponent->GetName(), *FoundTag.ToString());

    BroadcastTag(FoundTag);
}

FGameplayTag AAO_InspectionPuzzle::FindTagForComponent(UPrimitiveComponent* Component) const
{
    if (!Component)
    {
        return FGameplayTag();
    }

    FName ComponentName = Component->GetFName();

    for (const FAO_InspectionElementMapping& Mapping : ElementMappings)
    {
        if (Mapping.MeshComponentName == ComponentName)
        {
            return Mapping.ActivatedTag;
        }
    }

    return FGameplayTag();
}

void AAO_InspectionPuzzle::BroadcastTag(const FGameplayTag& Tag)
{
    if (!HasAuthority() || !LinkedChecker)
    {
        return;
    }

    LinkedChecker->OnPuzzleEvent(Tag, GetInstigator());
}

void AAO_InspectionPuzzle::ResetToInitialState()
{
	if (!HasAuthority()) return;

	bInteractionEnabled = true;
    
	// ElementMappings의 활성화 상태 초기화
	for (FAO_InspectionElementMapping& Mapping : ReplicatedMappings)
	{
		Mapping.bIsActivated = false;
	}
}

void AAO_InspectionPuzzle::SetInteractionEnabled(bool bEnabled)
{
	if (!HasAuthority()) return;
	bInteractionEnabled = bEnabled;
}