#include "AO/Public/AI/Character/Anim/AO_AnimInstnace_Crab.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UAO_AnimInstance_Crab::UAO_AnimInstance_Crab()
{
	Speed = 0.f;
	YawRate = 0.f;
	bIsTurningLeft = false;
	bIsTurningRight = false;
	PrevYaw = 0.f;
	CachedPawn = nullptr;
}

void UAO_AnimInstance_Crab::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!CachedPawn)
	{
		CachedPawn = TryGetPawnOwner();
		if (!CachedPawn)
		{
			return;
		}
		
		PrevYaw = CachedPawn->GetActorRotation().Yaw;
	}
	
	FVector Velocity = CachedPawn->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();
	
	const FRotator CurrentRot = CachedPawn->GetActorRotation();
	const float CurrentYaw = CurrentRot.Yaw;
	
	const FRotator DeltaRot = (CurrentRot - FRotator(0.f, PrevYaw, 0.f)).GetNormalized();
	const float DeltaYaw = DeltaRot.Yaw;
	YawRate = DeltaYaw / DeltaSeconds;
	
	const float TurnThreshold = 45.0f;
	const float MoveThreshold = 5.0f;

	const bool bIsStationary = Speed < MoveThreshold;
	bIsTurningLeft  = bIsStationary && (YawRate < -TurnThreshold);
	bIsTurningRight = bIsStationary && (YawRate >  TurnThreshold);

	PrevYaw = CurrentYaw;
}
