// KSJ : AO_Enemy_Crab.cpp

#include "AI/Character/AO_Enemy_Crab.h"
#include "AI/Item/AO_PickupComponent.h" // PickupComponent 사용
#include "AI/GAS/AO_Enemy_AttributeSet.h"
#include "AI/Manager/AO_Manager_CrabPickupManager.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "AO_Log.h"

AAO_Enemy_Crab::AAO_Enemy_Crab()
{
	PrimaryActorTick.bCanEverTick = false;

	PickupSocketName = FName("PickupSocket");
	IgnoreItemTag = FGameplayTag::RequestGameplayTag(FName("Item.State.Ignored"));
	HeldItemActor = nullptr;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0, 540.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;

	bUseControllerRotationYaw = false;
}

void AAO_Enemy_Crab::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AAO_Enemy_Crab, HeldItemActor);
}

void AAO_Enemy_Crab::GrabItem(AActor* ItemActor)
{
	if (!HasAuthority())
	{
		ServerGrabItem(ItemActor);
		return;
	}

	if (!ItemActor || HeldItemActor) return;
	
	UWorld* World = GetWorld();
	if (!World) return;
	
	UAO_Manager_CrabPickupManager* Manager = World->GetSubsystem<UAO_Manager_CrabPickupManager>();
	if (!Manager)
	{
		AO_LOG(LogKSJ, Error, TEXT("Crab GrabItem: CrabPickupManager not found!"));
		return;
	}
	
	if (!Manager->TryClaimItem(ItemActor, this))
	{
		AO_LOG(LogKSJ, Warning, TEXT("Crab GrabItem: Failed to claim item %s (already owned or in cooldown)"), *ItemActor->GetName());
		return;
	}
	
	UAO_PickupComponent* PickupComp = ItemActor->FindComponentByClass<UAO_PickupComponent>();
	if (!PickupComp)
	{
		Manager->ReleaseItem(ItemActor);
		AO_LOG(LogKSJ, Warning, TEXT("Crab GrabItem: ItemActor %s has no PickupComponent"), *ItemActor->GetName());
		return;
	}

	float DistanceSq = FVector::DistSquared(GetActorLocation(), ItemActor->GetActorLocation());
	float MaxGrabDistanceSq = 200.0f * 200.0f;
	if (DistanceSq > MaxGrabDistanceSq)
	{
		Manager->ReleaseItem(ItemActor);
		AO_LOG(LogKSJ, Warning, TEXT("Crab GrabItem: Item %s is too far away (Distance: %.1f)"), 
			*ItemActor->GetName(), FMath::Sqrt(DistanceSq));
		return;
	}

	bool bSuccess = PickupComp->TryPickup(GetMesh(), PickupSocketName);
	if (bSuccess)
	{
		HeldItemActor = ItemActor;
		SetMovementSpeed(BurdenedSpeed);
		
		AO_LOG(LogKSJ, Log, TEXT("Crab Grabbed Item (Speed -> %.1f)"), BurdenedSpeed);
	}
	else
	{
		Manager->ReleaseItem(ItemActor);
		AO_LOG(LogKSJ, Warning, TEXT("Crab GrabItem: TryPickup failed for %s"), *ItemActor->GetName());
	}
}

void AAO_Enemy_Crab::DropItem()
{
	if (!HasAuthority())
	{
		ServerDropItem();
		return;
	}

	if (!HeldItemActor) return;

	AActor* ItemToDrop = HeldItemActor;

	UAO_PickupComponent* PickupComp = HeldItemActor->FindComponentByClass<UAO_PickupComponent>();
	if (PickupComp)
	{
		if (IgnoreItemTag.IsValid())
		{
			PickupComp->AddTag(IgnoreItemTag);
		}
		PickupComp->TryDrop();
	}
	else
	{
		HeldItemActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	}

	AO_LOG(LogKSJ, Log, TEXT("Crab Dropped Item: %s"), *HeldItemActor->GetName());

	HeldItemActor = nullptr;
	
	UWorld* World = GetWorld();
	if (World)
	{
		if (UAO_Manager_CrabPickupManager* Manager = World->GetSubsystem<UAO_Manager_CrabPickupManager>())
		{
			Manager->ReleaseItem(ItemToDrop);
		}
	}
	
	SetMovementSpeed(NormalSpeed);
}

void AAO_Enemy_Crab::SetMovementSpeed(float NewSpeed)
{
	if (HasAuthority() && AttributeSet)
	{
		AttributeSet->SetMovementSpeed(NewSpeed);
	}
}

void AAO_Enemy_Crab::SetFleeMode(bool bIsFleeing)
{
	if (HeldItemActor)
	{
		SetMovementSpeed(BurdenedSpeed);
	}
	else
	{
		SetMovementSpeed(bIsFleeing ? FleeSpeed : NormalSpeed);
	}
}

void AAO_Enemy_Crab::ServerGrabItem_Implementation(AActor* ItemActor)
{
	GrabItem(ItemActor);
}

void AAO_Enemy_Crab::ServerDropItem_Implementation()
{
	DropItem();
}