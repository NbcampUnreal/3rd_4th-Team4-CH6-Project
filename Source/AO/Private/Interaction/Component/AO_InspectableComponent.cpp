// HSJ : AO_InspectableComponent.cpp
#include "Interaction/Component/AO_InspectableComponent.h"
#include "Net/UnrealNetwork.h"

UAO_InspectableComponent::UAO_InspectableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UAO_InspectableComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UAO_InspectableComponent, bIsLocked);
	DOREPLIFETIME(UAO_InspectableComponent, CurrentInspectingPlayer);
}

FTransform UAO_InspectableComponent::GetInspectionCameraTransform() const
{
	FTransform OwnerTransform = GetOwner()->GetActorTransform();
	FTransform RelativeTransform(CameraRelativeRotation, CameraRelativeLocation);
    
	return RelativeTransform * OwnerTransform;
}

void UAO_InspectableComponent::SetInspectionLocked(bool bLocked, AActor* InspectingPlayer)
{
	bIsLocked = bLocked;
	CurrentInspectingPlayer = InspectingPlayer;
}

bool UAO_InspectableComponent::IsLockedByOtherPlayer(AActor* QueryPlayer) const
{
	if (!bIsLocked)
	{
		return false;
	}

	return CurrentInspectingPlayer != QueryPlayer;
}