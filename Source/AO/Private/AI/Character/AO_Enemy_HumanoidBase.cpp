// AO_Enemy_HumanoidBase.cpp

#include "AI/Character/AO_Enemy_HumanoidBase.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"

AAO_Enemy_HumanoidBase::AAO_Enemy_HumanoidBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void AAO_Enemy_HumanoidBase::BeginPlay()
{
	Super::BeginPlay();

	ApplyMovementSpeed();
}

void AAO_Enemy_HumanoidBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAO_Enemy_HumanoidBase, MovementState);
	DOREPLIFETIME(AAO_Enemy_HumanoidBase, CurrentTarget);
	DOREPLIFETIME(AAO_Enemy_HumanoidBase, LastSeenLocation);
	DOREPLIFETIME(AAO_Enemy_HumanoidBase, bCanAttack);
}

void AAO_Enemy_HumanoidBase::OnRep_MovementState()
{
	ApplyMovementSpeed();
}

void AAO_Enemy_HumanoidBase::ApplyMovementSpeed()
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp)
	{
		return;
	}

	switch(MovementState)
	{
	case EEnemyMovementState::Idle:
	case EEnemyMovementState::Patrol:
		MoveComp->MaxWalkSpeed = WalkSpeed;
		break;
	case EEnemyMovementState::Chase:
		MoveComp->MaxWalkSpeed = ChaseSpeed;
		break;
	}
}

void AAO_Enemy_HumanoidBase::SetMovementState(EEnemyMovementState NewState)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		MovementState = NewState;
		ApplyMovementSpeed();
	}
}

void AAO_Enemy_HumanoidBase::SetCurrentTarget(AActor* NewTarget)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		CurrentTarget = NewTarget;
	}
}

void AAO_Enemy_HumanoidBase::SetLastSeenLocation(const FVector& Location)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		LastSeenLocation = Location;
	}
}

void AAO_Enemy_HumanoidBase::SetCanAttack(bool bValue)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		bCanAttack = bValue;
	}
}








