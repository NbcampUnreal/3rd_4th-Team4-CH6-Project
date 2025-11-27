// KSJ : AO_PickupComponent.cpp

#include "AI/Item/AO_PickupComponent.h"
#include "Net/UnrealNetwork.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"

UAO_PickupComponent::UAO_PickupComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UAO_PickupComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UAO_PickupComponent, bIsPickedUp);
	DOREPLIFETIME(UAO_PickupComponent, ItemTags);
}

bool UAO_PickupComponent::TryPickup(USceneComponent* AttachTo, FName SocketName)
{
	if (!AttachTo || bIsPickedUp) return false;
	
	if (IsInCooldown()) return false;

	AActor* Owner = GetOwner();
	if (!Owner) return false;

	// 물리 끄기
	ConfigurePhysics(false);

	// 부착
	bool bAttached = Owner->AttachToComponent(AttachTo, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
	if (bAttached)
	{
		bIsPickedUp = true;
		CooldownExpireTime = 0.0;
	}
	else
	{
		// 실패 시 물리 복구
		ConfigurePhysics(true);
	}

	return bAttached;
}

void UAO_PickupComponent::TryDrop()
{
	if (!bIsPickedUp) return;

	AActor* Owner = GetOwner();
	if (!Owner) return;

	// 부착 해제
	Owner->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	// 물리 켜기
	ConfigurePhysics(true);

	bIsPickedUp = false;
	
	SetIgnoreCooldown(5.0f);
}

bool UAO_PickupComponent::HasTag(FGameplayTag TagToCheck) const
{
	return ItemTags.HasTag(TagToCheck);
}

void UAO_PickupComponent::AddTag(FGameplayTag TagToAdd)
{
	if (TagToAdd.IsValid())
	{
		ItemTags.AddTag(TagToAdd);
	}
}

void UAO_PickupComponent::OnRep_IsPickedUp()
{
	ConfigurePhysics(!bIsPickedUp);
}

void UAO_PickupComponent::ConfigurePhysics(bool bEnablePhysics)
{
	if (!TargetPhysicsComp)
	{
		// 첫 호출 시 RootComponent 찾기
		if (AActor* Owner = GetOwner())
		{
			TargetPhysicsComp = Cast<UPrimitiveComponent>(Owner->GetRootComponent());
		}
	}

	if (TargetPhysicsComp)
	{
		TargetPhysicsComp->SetSimulatePhysics(bEnablePhysics);
		if (bEnablePhysics)
		{
			TargetPhysicsComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		}
		else
		{
			TargetPhysicsComp->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 혹은 QueryOnly
		}
	}
}

void UAO_PickupComponent::SetIgnoreCooldown(float Duration)
{
	UWorld* World = GetWorld();
	if (World)
	{
		CooldownExpireTime = World->GetTimeSeconds() + Duration;
	}
}

bool UAO_PickupComponent::IsInCooldown() const
{
	UWorld* World = GetWorld();
	if (World)
	{
		return World->GetTimeSeconds() < CooldownExpireTime;
	}
	return false;
}