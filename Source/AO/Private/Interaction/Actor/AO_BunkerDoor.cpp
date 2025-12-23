// HSJ : AO_BunkerDoor.cpp
#include "Interaction/Actor/AO_BunkerDoor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

AAO_BunkerDoor::AAO_BunkerDoor(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    if (MeshComponent)
    {
        MeshComponent->SetVisibility(false);
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    DoorSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("DoorSkeletalMesh"));
    DoorSkeletalMesh->SetupAttachment(RootComponent);
    DoorSkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    DoorSkeletalMesh->SetCollisionResponseToAllChannels(ECR_Block);
    
    DoorSkeletalMesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);

    bIsToggleable = true;
    
    InteractionTitle = FText::FromString(TEXT("벙커 문"));
    InteractionContent = FText::FromString(TEXT("열기/닫기"));
}

bool AAO_BunkerDoor::CanInteraction(const FAO_InteractionQuery& InteractionQuery) const
{
	if (!Super::CanInteraction(InteractionQuery))
	{
		return false;
	}

	if (bIsAnimating)
	{
		return false;
	}

	return true;
}

void AAO_BunkerDoor::BeginPlay()
{
    Super::BeginPlay();

    if (DoorSkeletalMesh && DoorAnimation)
    {
        DoorSkeletalMesh->PlayAnimation(DoorAnimation, false);
        DoorSkeletalMesh->SetPosition(0.0f);
        DoorSkeletalMesh->Stop();
    }

    bDoorOpen = false;
    bIsAnimating = false;
}

void AAO_BunkerDoor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    TObjectPtr<UWorld> World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(DoorAnimationCheckTimer);
    }
    
    Super::EndPlay(EndPlayReason);
}

void AAO_BunkerDoor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(AAO_BunkerDoor, bDoorOpen);
    DOREPLIFETIME(AAO_BunkerDoor, bIsAnimating);
}

void AAO_BunkerDoor::OnInteractionSuccess_BP_Implementation(AActor* Interactor)
{
    Super::OnInteractionSuccess_BP_Implementation(Interactor);

    if (!HasAuthority())
    {
    	return;
    }

    if (bIsAnimating)
    {
    	return;
    }

    bDoorOpen = !bDoorOpen;

    if (bDoorOpen)
    {
        OpenDoor();
    }
    else
    {
        CloseDoor();
    }
}

void AAO_BunkerDoor::OpenDoor()
{
    bIsAnimating = true;
    
    if (DoorOpenSound && HasAuthority())
    {
        UGameplayStatics::PlaySoundAtLocation(this, DoorOpenSound, GetActorLocation());
    }

    if (DoorSkeletalMesh && DoorAnimation)
    {
        DoorSkeletalMesh->PlayAnimation(DoorAnimation, false);
        DoorSkeletalMesh->SetPlayRate(DoorAnimationSpeed);
        
        TObjectPtr<UWorld> World = GetWorld();
        if (World)
        {
            TWeakObjectPtr<AAO_BunkerDoor> WeakThis(this);
            World->GetTimerManager().SetTimer(
                DoorAnimationCheckTimer,
                FTimerDelegate::CreateWeakLambda(this, [WeakThis]()
                {
                    if (TObjectPtr<AAO_BunkerDoor> StrongThis = WeakThis.Get())
                    {
                        StrongThis->CheckDoorAnimation();
                    }
                }),
                0.016f,
                true
            );
        }
    }
}

void AAO_BunkerDoor::CloseDoor()
{
    bIsAnimating = true;
    
    if (DoorCloseSound && HasAuthority())
    {
        UGameplayStatics::PlaySoundAtLocation(this, DoorCloseSound, GetActorLocation());
    }

    if (DoorSkeletalMesh && DoorAnimation)
    {
        DoorSkeletalMesh->PlayAnimation(DoorAnimation, false);
        
        if (DoorAnimation)
        {
            float AnimLength = DoorAnimation->GetPlayLength();
            DoorSkeletalMesh->SetPosition(AnimLength);
        }
        
        DoorSkeletalMesh->SetPlayRate(-DoorAnimationSpeed);
        
        TObjectPtr<UWorld> World = GetWorld();
        if (World)
        {
            TWeakObjectPtr<AAO_BunkerDoor> WeakThis(this);
            World->GetTimerManager().SetTimer(
                DoorAnimationCheckTimer,
                FTimerDelegate::CreateWeakLambda(this, [WeakThis]()
                {
                    if (TObjectPtr<AAO_BunkerDoor> StrongThis = WeakThis.Get())
                    {
                        StrongThis->CheckDoorAnimation();
                    }
                }),
                0.016f,
                true
            );
        }
    }
}

void AAO_BunkerDoor::CheckDoorAnimation()
{
    if (!DoorSkeletalMesh || !DoorSkeletalMesh->IsPlaying())
    {
        bIsAnimating = false;
        
        TObjectPtr<UWorld> World = GetWorld();
        if (World)
        {
            World->GetTimerManager().ClearTimer(DoorAnimationCheckTimer);
        }
    }
}

void AAO_BunkerDoor::OnRep_DoorState()
{
    if (!HasAuthority())
    {
        if (bDoorOpen)
        {
            OpenDoor();
        }
        else
        {
            CloseDoor();
        }
    }
}