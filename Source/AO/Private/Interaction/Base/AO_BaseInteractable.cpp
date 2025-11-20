// HSJ : AO_BaseInteractable.cpp
#include "Interaction/Base/AO_BaseInteractable.h"
#include "Components/StaticMeshComponent.h"
#include "Physics/AO_CollisionChannels.h"
#include "Interaction/Base/GA_Interact_Base.h"

AAO_BaseInteractable::AAO_BaseInteractable(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bReplicates = true;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetCollisionObjectType(ECC_WorldDynamic);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
	MeshComponent->SetCollisionResponseToChannel(AO_TraceChannel_Interaction, ECR_Block);
}

FAO_InteractionInfo AAO_BaseInteractable::GetInteractionInfo(const FAO_InteractionQuery& InteractionQuery) const
{
	FAO_InteractionInfo Info;
	Info.Title = InteractionTitle;
	Info.Content = InteractionContent;
	Info.Duration = InteractionDuration;
	Info.AbilityToGrant = UGA_Interact_Base::StaticClass();
	return Info;
}

void AAO_BaseInteractable::GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const
{
	if (MeshComponent)
	{
		OutMeshComponents.Add(MeshComponent);
	}
}

void AAO_BaseInteractable::OnInteractionSuccess(AActor* Interactor)
{
	Super::OnInteractionSuccess(Interactor);

	OnInteractionSuccess_BP(Interactor);
}

void AAO_BaseInteractable::OnInteractionSuccess_BP_Implementation(AActor* Interactor)
{
	// 기본 구현은 비어있음, 오버라이드 가능
}