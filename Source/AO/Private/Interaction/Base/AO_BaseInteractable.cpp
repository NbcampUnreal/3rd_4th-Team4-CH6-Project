// HSJ : AO_BaseInteractable.cpp
#include "Interaction/Base/AO_BaseInteractable.h"
#include "Components/StaticMeshComponent.h"
#include "Physics/AO_CollisionChannels.h"
#include "Interaction/Base/GA_Interact_Base.h"
#include "Net/UnrealNetwork.h"
#include "Puzzle/Actor/AO_PuzzleReactionActor.h"

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

	InteractableMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("InteractableMeshComponent"));
	InteractableMeshComponent->SetupAttachment(MeshComponent);
	InteractableMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractableMeshComponent->SetCollisionObjectType(ECC_WorldDynamic);
	InteractableMeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
}

FAO_InteractionInfo AAO_BaseInteractable::GetInteractionInfo(const FAO_InteractionQuery& InteractionQuery) const
{
	FAO_InteractionInfo Info;
	Info.Title = InteractionTitle;
	Info.Content = InteractionContent;
	Info.Duration = InteractionDuration;
	Info.AbilityToGrant = UGA_Interact_Base::StaticClass();
	Info.ActiveHoldMontage  = ActiveHoldMontage;
	Info.ActiveMontage  = ActiveMontage;
	Info.DeactivateMontage = DeactivateMontage;
	Info.InteractionTransform = GetInteractionTransform();
	Info.WarpTargetName = WarpTargetName;
	Info.bWaitForAnimationNotify = bWaitForAnimationNotify;
	return Info;
}

void AAO_BaseInteractable::GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const
{
	if (!MeshComponent)
	{
		return;
	}

	// 루트 메시가 실제 메시를 가지고 있으면 추가
	if (TObjectPtr<UStaticMeshComponent> StaticMesh = Cast<UStaticMeshComponent>(MeshComponent))
	{
		if (StaticMesh->GetStaticMesh())
		{
			OutMeshComponents.Add(MeshComponent);
		}
	}
	else if (TObjectPtr<USkeletalMeshComponent> SkeletalMesh = Cast<USkeletalMeshComponent>(MeshComponent))
	{
		if (SkeletalMesh->GetSkeletalMeshAsset())
		{
			OutMeshComponents.Add(MeshComponent);
		}
	}

	// 자식 메시 컴포넌트들도 모두 수집
	TArray<USceneComponent*> ChildComponents;
	MeshComponent->GetChildrenComponents(true, ChildComponents);
    
	for (TObjectPtr<USceneComponent> Child : ChildComponents)
	{
		if (TObjectPtr<UMeshComponent> ChildMesh = Cast<UMeshComponent>(Child))
		{
			// Static Mesh 체크
			if (TObjectPtr<UStaticMeshComponent> StaticMesh = Cast<UStaticMeshComponent>(ChildMesh))
			{
				if (StaticMesh->GetStaticMesh())
				{
					OutMeshComponents.Add(ChildMesh);
				}
			}
			// Skeletal Mesh 체크
			else if (TObjectPtr<USkeletalMeshComponent> SkeletalMesh = Cast<USkeletalMeshComponent>(ChildMesh))
			{
				if (SkeletalMesh->GetSkeletalMeshAsset())
				{
					OutMeshComponents.Add(ChildMesh);
				}
			}
		}
	}
}

bool AAO_BaseInteractable::CanInteraction(const FAO_InteractionQuery& InteractionQuery) const
{
	return CanInteraction_BP(InteractionQuery);
}

bool AAO_BaseInteractable::CanInteraction_BP_Implementation(const FAO_InteractionQuery& InteractionQuery) const
{
	if (!Super::CanInteraction(InteractionQuery))
	{
		return false;
	}

	// 전역 비활성화 체크
	if (!bInteractionEnabled)
	{
		return false;
	}
	// 로컬 비활성화 체크
	if (!bLocalInteractionEnabled)
	{
		return false;
	}

	// 기본적으로 허용
	return true;
}

void AAO_BaseInteractable::OnInteractionSuccess(AActor* Interactor)
{
	Super::OnInteractionSuccess(Interactor);

	if (!HasAuthority())
	{
		return;
	}

	MulticastPlayInteractionSound();
	
	if (bIsToggleable)
	{
		bIsActivated = !bIsActivated;
		StartInteractionAnimation(bIsActivated);
		TriggerLinkedReactions(bIsActivated);
	}
	else
	{
		bIsActivated = true;
		StartInteractionAnimation(true);
		TriggerLinkedReactions(true);
	}

	OnInteractionSuccess_BP(Interactor);
	MulticastNotifyInteractionSuccess(Interactor);
}

void AAO_BaseInteractable::MulticastNotifyInteractionSuccess_Implementation(AActor* Interactor)
{
	OnInteractionSuccessMulticast_BP(Interactor);
}

void AAO_BaseInteractable::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, bIsActivated);
	DOREPLIFETIME(ThisClass, bInteractionEnabled);
}

void AAO_BaseInteractable::OnInteractionSuccess_BP_Implementation(AActor* Interactor)
{
	// 기본 구현은 비어있음, 오버라이드 가능
}

void AAO_BaseInteractable::OnRep_IsActivated()
{
	StartInteractionAnimation(bIsActivated);
}

void AAO_BaseInteractable::TriggerLinkedReactions(bool bActivate)
{
	if (!HasAuthority())
	{
		return;
	}
    
	for (TObjectPtr<AAO_PuzzleReactionActor> ReactionActor : LinkedReactionActors)
	{
		if (!ReactionActor)
		{
			continue;
		}
        
		if (bActivate)
		{
			ReactionActor->ActivateReaction();
		}
		else
		{
			ReactionActor->DeactivateReaction();
		}
	}
}

FTransform AAO_BaseInteractable::GetInteractionTransform() const
{
	if (!MeshComponent || InteractionSocketName.IsNone())
	{
		return GetActorTransform();
	}

	// 루트 메시에서 먼저 Socket 찾기
	if (MeshComponent->DoesSocketExist(InteractionSocketName))
	{
		return MeshComponent->GetSocketTransform(InteractionSocketName);
	}

	// 자식 메시들에서 Socket 찾기
	TArray<USceneComponent*> ChildComponents;
	MeshComponent->GetChildrenComponents(true, ChildComponents);
    
	for (TObjectPtr<USceneComponent> Child : ChildComponents)
	{
		if (TObjectPtr<UMeshComponent> ChildMesh = Cast<UMeshComponent>(Child))
		{
			if (ChildMesh->DoesSocketExist(InteractionSocketName))
			{
				return ChildMesh->GetSocketTransform(InteractionSocketName);
			}
		}
	}

	return GetActorTransform();
}
