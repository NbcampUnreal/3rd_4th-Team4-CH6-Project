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
    // Owner 액터의 모든 메시 컴포넌트를 하이라이트 대상으로 반환
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    TArray<UMeshComponent*> MeshComponents;
    Owner->GetComponents<UMeshComponent>(MeshComponents);
    
    for (UMeshComponent* MeshComp : MeshComponents)
    {
        if (MeshComp)
        {
            OutMeshComponents.Add(MeshComp);
        }
    }
}

// 상호작용 성공 시 호출
void UAO_InteractableComponent::NotifyInteractionSuccess(AActor* Interactor)
{
    AActor* Owner = GetOwner();
    if (!Owner || !Owner->HasAuthority())
    {
        return;
    }

    // BP 이벤트 호출
    OnInteractionSuccess.Broadcast(Interactor);
}