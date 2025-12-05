// HSJ : AO_InteractableComponent.cpp
#include "Interaction/Component/AO_InteractableComponent.h"
#include "Interaction/Base/GA_Interact_Base.h"
#include "Components/MeshComponent.h"
#include "Net/UnrealNetwork.h"

UAO_InteractableComponent::UAO_InteractableComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);

    InteractionTitle = FText::FromString("Interact");
    InteractionContent = FText::FromString(": Press F");
}

void UAO_InteractableComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(UAO_InteractableComponent, bInteractionEnabled);
}

FAO_InteractionInfo UAO_InteractableComponent::GetInteractionInfo(const FAO_InteractionQuery& InteractionQuery) const
{
    FAO_InteractionInfo Info;
    Info.Title = InteractionTitle;
    Info.Content = InteractionContent;
    Info.Duration = 0.0f;
    Info.AbilityToGrant = UGA_Interact_Base::StaticClass();
	Info.ActiveHoldMontage = ActiveHoldMontage;
	Info.ActiveMontage = ActiveMontage;
	Info.InteractionTransform = GetInteractionTransform();
	Info.WarpTargetName = WarpTargetName;
    return Info;
}

bool UAO_InteractableComponent::CanInteraction(const FAO_InteractionQuery& InteractionQuery) const
{
    // 상호작용 비활성화
    if (!bInteractionEnabled)
    {
        return false;
    }

    return true;
}

void UAO_InteractableComponent::GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const
{
	TObjectPtr<AActor> Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Owner의 모든 메시 컴포넌트 수집 (재귀적으로)
	TArray<UMeshComponent*> AllMeshComponents;
	Owner->GetComponents<UMeshComponent>(AllMeshComponents, true); // true = 자식까지
    
	for (TObjectPtr<UMeshComponent> MeshComp : AllMeshComponents)
	{
		if (!MeshComp) continue;
        
		// Static Mesh 체크
		if (TObjectPtr<UStaticMeshComponent> StaticMesh = Cast<UStaticMeshComponent>(MeshComp))
		{
			if (StaticMesh->GetStaticMesh())
			{
				OutMeshComponents.Add(MeshComp);
			}
		}
		// Skeletal Mesh 체크
		else if (TObjectPtr<USkeletalMeshComponent> SkeletalMesh = Cast<USkeletalMeshComponent>(MeshComp))
		{
			if (SkeletalMesh->GetSkeletalMeshAsset())
			{
				OutMeshComponents.Add(MeshComp);
			}
		}
	}
}

// 상호작용 성공 시 호출
void UAO_InteractableComponent::NotifyInteractionSuccess(AActor* Interactor)
{
    TObjectPtr<AActor> Owner = GetOwner();
    if (!Owner || !Owner->HasAuthority())
    {
        return;
    }

    // BP 이벤트 호출
    OnInteractionSuccess.Broadcast(Interactor);
}

FTransform UAO_InteractableComponent::GetInteractionTransform() const
{
	TObjectPtr<AActor> Owner = GetOwner();
	if (!Owner) return FTransform::Identity;
    
	if (!InteractionSocketName.IsNone())
	{
		// Owner의 모든 메시 컴포넌트에서 Socket 찾기 (재귀)
		TArray<UMeshComponent*> MeshComponents;
		Owner->GetComponents<UMeshComponent>(MeshComponents, true);
        
		for (TObjectPtr<UMeshComponent> MeshComp : MeshComponents)
		{
			if (MeshComp && MeshComp->DoesSocketExist(InteractionSocketName))
			{
				return MeshComp->GetSocketTransform(InteractionSocketName);
			}
		}
	}
    
	return Owner->GetActorTransform();
}
