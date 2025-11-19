#include "Item/AO_ItemPickupComponent.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"

UAO_ItemPickupComponent::UAO_ItemPickupComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UAO_ItemPickupComponent::BeginPlay()
{
    Super::BeginPlay();

    // Owner에서 컴포넌트 자동 탐색
    MeshComponent = GetOwner()->FindComponentByClass<UStaticMeshComponent>();
    SphereComponent = GetOwner()->FindComponentByClass<USphereComponent>();

    if (!MeshComponent)
        UE_LOG(LogTemp, Error, TEXT("PickupComponent: MeshComponent not found on %s"), *GetOwner()->GetName());
    if (!SphereComponent)
        UE_LOG(LogTemp, Error, TEXT("PickupComponent: SphereComponent not found on %s"), *GetOwner()->GetName());
}

// ---------------- Gameplay Tags ----------------
void UAO_ItemPickupComponent::AddGameplayTag(FGameplayTag Tag)
{
    if (GetOwner() && GetOwner()->HasAuthority() && Tag.IsValid())
    {
        ItemTags.AddTag(Tag);
    }
}

void UAO_ItemPickupComponent::RemoveGameplayTag(FGameplayTag Tag)
{
    if (GetOwner() && GetOwner()->HasAuthority() && Tag.IsValid())
    {
        ItemTags.RemoveTag(Tag);
    }
}

void UAO_ItemPickupComponent::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
    TagContainer = ItemTags;
}

// ---------------- Pickup Logic ----------------
void UAO_ItemPickupComponent::Pickup(USceneComponent* AttachTo, FName SocketName)
{
    if (!GetOwner() || !GetOwner()->HasAuthority() || bIsPickedUp || !AttachTo)
        return;

    bIsPickedUp = true;
    HolderActor = AttachTo->GetOwner();

    ApplyPickedUpVisuals();
}

void UAO_ItemPickupComponent::Drop()
{
    if (!GetOwner() || !GetOwner()->HasAuthority() || !bIsPickedUp)
        return;

    bIsPickedUp = false;
    HolderActor = nullptr;

    ApplyDroppedVisuals();
}

// ---------------- Repnotify ----------------
void UAO_ItemPickupComponent::OnRep_Holder()
{
    if (bIsPickedUp)
        ApplyPickedUpVisuals();
}

void UAO_ItemPickupComponent::OnRep_IsPickedUp()
{
    if (bIsPickedUp)
        ApplyPickedUpVisuals();
    else
        ApplyDroppedVisuals();
}

// ---------------- Visual Logic ----------------
void UAO_ItemPickupComponent::ApplyPickedUpVisuals()
{
    if (!MeshComponent || !SphereComponent) return;

    MeshComponent->SetSimulatePhysics(false);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    if (!HolderActor) return;

    USkeletalMeshComponent* HolderMesh = HolderActor->FindComponentByClass<USkeletalMeshComponent>();
    static const FName DefaultSocket(TEXT("PickupSocket"));

    if (HolderMesh && HolderMesh->DoesSocketExist(DefaultSocket))
    {
        GetOwner()->AttachToComponent(HolderMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, DefaultSocket);
    }
}

void UAO_ItemPickupComponent::ApplyDroppedVisuals()
{
    if (!MeshComponent || !SphereComponent) return;

    GetOwner()->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    MeshComponent->SetSimulatePhysics(bDropWithPhysics);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

    if (bDropWithPhysics)
    {
        MeshComponent->SetEnableGravity(true);
        MeshComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
        MeshComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
        MeshComponent->RecreatePhysicsState();
    }
}

// ---------------- Replication ----------------
void UAO_ItemPickupComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UAO_ItemPickupComponent, bIsPickedUp);
    DOREPLIFETIME(UAO_ItemPickupComponent, HolderActor);
    DOREPLIFETIME(UAO_ItemPickupComponent, ItemTags);
}
