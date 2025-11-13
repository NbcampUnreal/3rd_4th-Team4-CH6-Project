// AO_Enemy_Crab.cpp

#include "AO/Public/AI/Character/AO_Enemy_Crab.h"
#include "AO/Public/AI/Item/AO_TestPickupItem.h"
#include "GameFramework/CharacterMovementComponent.h"

AAO_Enemy_Crab::AAO_Enemy_Crab()
{
	PrimaryActorTick.bCanEverTick = false;

	WalkSpeed = 220.0f;
	SneakSpeed = 140.0f;

	PickupSocketName = FName("PickupSocket");
	HeldItem = nullptr;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0,540.f,0);

	UseWalkSpeed();

	bUseControllerRotationYaw = false;
}

void AAO_Enemy_Crab::UseWalkSpeed()
{
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void AAO_Enemy_Crab::UseSneakSpeed()
{
	GetCharacterMovement()->MaxWalkSpeed = SneakSpeed;
}

void AAO_Enemy_Crab::GrabItem(AAO_TestPickupItem* Item)
{
	if (!HasAuthority())
	{
		return;
	}
	if (HeldItem != nullptr || Item == nullptr)
	{
		return;
	}

	if (!GetMesh()->DoesSocketExist(PickupSocketName))
	{
		return;
	}

	Item->Pickup(GetMesh(), PickupSocketName);

	const bool bAttachedToMe = (Item->GetAttachParentActor() == this);
	if (Item->bIsPickedUp && bAttachedToMe)
	{
		HeldItem = Item;
	}
}

void AAO_Enemy_Crab::DropItem()
{
	if (!HasAuthority())
	{
		return;
	}
	if (HeldItem != nullptr)
	{
		return;
	}

	HeldItem->Drop();
	HeldItem = nullptr;
}



