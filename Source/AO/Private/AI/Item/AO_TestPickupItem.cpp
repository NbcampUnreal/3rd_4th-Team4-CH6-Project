#include "AO/Public/AI/Item/AO_TestPickupItem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/EngineTypes.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"

AAO_TestPickupItem::AAO_TestPickupItem()
{
    PrimaryActorTick.bCanEverTick = false;

    SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
    SphereComponent->InitSphereRadius(50.0f);
    RootComponent = SphereComponent;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);

    // ── Replication ──────────────────────────────────────────────
    bReplicates = true;
    SetReplicateMovement(true);
    MeshComponent->SetIsReplicated(true);
    
    MeshComponent->SetCanEverAffectNavigation(false);
    SphereComponent->SetCanEverAffectNavigation(false);
    
    MeshComponent->SetSimulatePhysics(false);
    SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    bIsPickedUp = false;
}

void AAO_TestPickupItem::BeginPlay()
{
    Super::BeginPlay();
}

void AAO_TestPickupItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AAO_TestPickupItem, bIsPickedUp);
    DOREPLIFETIME(AAO_TestPickupItem, HolderActor);
    DOREPLIFETIME(AAO_TestPickupItem, ItemTags);
}

// ── Gameplay Tags ───────────────────────────────────────────────
void AAO_TestPickupItem::AddGameplayTag(FGameplayTag Tag)
{
    if (HasAuthority() && Tag.IsValid())
    {
        ItemTags.AddTag(Tag);
    }
}

void AAO_TestPickupItem::RemoveGameplayTag(FGameplayTag Tag)
{
    if (HasAuthority() && Tag.IsValid())
    {
        ItemTags.RemoveTag(Tag);
    }
}

void AAO_TestPickupItem::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
    TagContainer = ItemTags;
}

// ── Pickup / Drop ───────────────────────────────────────────────
void AAO_TestPickupItem::Pickup(USceneComponent* AttachTo, FName SocketName)
{
    if (!HasAuthority() || bIsPickedUp || AttachTo == nullptr)
    {
        return;
    }

    if (const USkeletalMeshComponent* Skel = Cast<USkeletalMeshComponent>(AttachTo))
    {
        if (!Skel->DoesSocketExist(SocketName))
        {
            UE_LOG(LogTemp, Warning, TEXT("Pickup failed: Socket %s not exist on %s"),
                   *SocketName.ToString(), *Skel->GetName());
            return;
        }
    }
    
    bIsPickedUp = true;
    HolderActor = AttachTo->GetOwner();
    
    ApplyPickedUpVisuals();
}

void AAO_TestPickupItem::Drop()
{
    if (!HasAuthority() || !bIsPickedUp)
    {
        return;
    }

    bIsPickedUp = false;
    HolderActor = nullptr;

    ApplyDroppedVisuals();
}

// ── RepNotifies ─────────────────────────────────────────────────
void AAO_TestPickupItem::OnRep_Holder()
{
    if (bIsPickedUp)
    {
        ApplyPickedUpVisuals();
    }
}

void AAO_TestPickupItem::OnRep_IsPickedUp()
{
    if (bIsPickedUp)
    {
        ApplyPickedUpVisuals();
    }
    else
    {
        ApplyDroppedVisuals();
    }
}

// ── Visual helpers (서버/클라 공용) ─────────────────────────────
void AAO_TestPickupItem::ApplyPickedUpVisuals()
{
    MeshComponent->SetSimulatePhysics(false);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    
    if (HolderActor)
    {
        USkeletalMeshComponent* HolderMesh = HolderActor->FindComponentByClass<USkeletalMeshComponent>();
        static const FName DefaultSocket(TEXT("PickupSocket"));
        const FName SocketName = DefaultSocket;

        if (HolderMesh && HolderMesh->DoesSocketExist(SocketName))
        {
            AttachToComponent(HolderMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, SocketName);
        }
    }
}

void AAO_TestPickupItem::ApplyDroppedVisuals()
{
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    
    const FRotator Upright(0.f, GetActorRotation().Yaw, 0.f);
    SetActorRotation(Upright);
    
    const FVector Start = GetActorLocation() + FVector(0, 0, 50.f);
    const FVector End   = Start - FVector(0, 0, 500.f);
    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(DropSnap), false, this);
    GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

    const float HalfHeight = MeshComponent->Bounds.BoxExtent.Z;
    const float Margin = 1.0f;

    FVector Desired = GetActorLocation();
    if (Hit.bBlockingHit)
    {
        Desired = Hit.ImpactPoint + FVector(0, 0, HalfHeight + Margin);
    }

    FHitResult SweepHit;
    SetActorLocation(Desired, true, &SweepHit);

    if (SweepHit.bStartPenetrating)
    {
        const FVector Adjust = SweepHit.Normal * (SweepHit.PenetrationDepth + 1.f);
        SetActorLocation(Desired + Adjust, false);
    }
    
    MeshComponent->SetSimulatePhysics(false);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    
    if (bDropWithPhysics)
    {
        MeshComponent->SetSimulatePhysics(true);
        MeshComponent->SetEnableGravity(true);
        MeshComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
        MeshComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
        MeshComponent->BodyInstance.bLockXRotation = true;
        MeshComponent->BodyInstance.bLockYRotation = true;
        MeshComponent->BodyInstance.bLockZRotation = false;
        MeshComponent->BodyInstance.SetDOFLock(EDOFMode::Default);
        MeshComponent->RecreatePhysicsState();
    }
}
