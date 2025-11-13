// AO_PlayerCharacter_AnimInstance.cpp

#include "Character/AO_PlayerCharacter_AnimInstance.h"

#include "AO/AO_Log.h"
#include "Character/AO_PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PoseSearch/PoseSearchTrajectoryLibrary.h"
#include "Kismet/KismetMathLibrary.h"

void FAOAnimInstanceProxy::InitializeObjects(UAnimInstance* InAnimInstance)
{
	FAnimInstanceProxy::InitializeObjects(InAnimInstance);

	if (!InAnimInstance) return;

	// 프리뷰 월드 방지
	if (!InAnimInstance->GetWorld() || InAnimInstance->GetWorld()->IsPreviewWorld()) return;
	
	Owner = InAnimInstance->TryGetPawnOwner();
	if (!Owner) return;

	Character = Cast<AAO_PlayerCharacter>(Owner);
	if (!Character) return;
	
	MovementComponent = Character->GetCharacterMovement();
}

void FAOAnimInstanceProxy::PreUpdate(UAnimInstance* InAnimInstance, float DeltaSeconds)
{
	FAnimInstanceProxy::PreUpdate(InAnimInstance, DeltaSeconds);
}

void FAOAnimInstanceProxy::Update(float DeltaSeconds)
{
	FAnimInstanceProxy::Update(DeltaSeconds);
}

void UAO_PlayerCharacter_AnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	TrajectoryData_Idle.RotateTowardsMovementSpeed = 0.f;
	TrajectoryData_Idle.MaxControllerYawRate = 100.f;
	TrajectoryData_Moving.RotateTowardsMovementSpeed = 0.f;
	TrajectoryData_Moving.MaxControllerYawRate = 0.f;
}

void UAO_PlayerCharacter_AnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
}

void UAO_PlayerCharacter_AnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
	if (!Proxy.Owner || !Proxy.Character || !Proxy.MovementComponent) return;

	UpdateTrajectory(DeltaSeconds);
	UpdateEssentialValues();
	UpdateStates();
}

bool UAO_PlayerCharacter_AnimInstance::IsMoving() 
{
	if (TrjFutureVelocity.IsNearlyZero(10.f) && Acceleration.IsNearlyZero())
	{
		return false;
	}
	
	return true;
}

bool UAO_PlayerCharacter_AnimInstance::IsStarting() 
{
	if (IsMoving()
		&& TrjFutureVelocity.Size2D() >= Velocity.Size2D()
		&& !CurrentDatabaseTags.Contains(FName(TEXT("Pivots"))))
	{
		return true; 
	}
	
	return false;
}

bool UAO_PlayerCharacter_AnimInstance::IsPivoting() 
{
	if (IsMoving())
	{
		return true;
	}
	
	return false;
}

bool UAO_PlayerCharacter_AnimInstance::ShouldTurnInPlace()
{
	if (MovementState == EMovementState::Idle
		&& MovementState == EMovementState::Moving)
	{
		return true;
	}

	return false;
}

bool UAO_PlayerCharacter_AnimInstance::ShouldSpinTransition()
{
	if (Speed2D >= 150.f
		&& !CurrentDatabaseTags.Contains(FName(TEXT("Pivots"))))
	{
		return true;
	}

	return false;
}

bool UAO_PlayerCharacter_AnimInstance::JustTraversal()
{
	if (!IsSlotActive(FName(TEXT("DefaultSlot")))
		&& GetCurveValue(FName(TEXT("MovingTraversal"))) > 0.0f
		&& abs(GetTrajectoryTurnAngle()) < 50.f)
	{
		return true;
	}
	
	return false;
}

bool UAO_PlayerCharacter_AnimInstance::JustLanded_Heavy()
{
	const AAO_PlayerCharacter* CachedCharacter = Proxy.Character;
	if (!CachedCharacter)
	{
		return false;
	}
	
	return (CachedCharacter->bJustLanded
		&& abs(CachedCharacter->LandVelocity.Z) >= abs(HeavyLandSpeedThreshold));
}

bool UAO_PlayerCharacter_AnimInstance::JustLanded_Light()
{
	const AAO_PlayerCharacter* CachedCharacter = Proxy.Character;
	if (!CachedCharacter)
	{
		return false;
	}
	
	return (CachedCharacter->bJustLanded
		&& abs(CachedCharacter->LandVelocity.Z) < abs(HeavyLandSpeedThreshold));
}

float UAO_PlayerCharacter_AnimInstance::GetTrajectoryTurnAngle()
{
	FRotator CurrentRotation = UKismetMathLibrary::MakeRotFromX(Velocity);
	FRotator FutureRotation = UKismetMathLibrary::MakeRotFromX(TrjFutureVelocity);
	
	return UKismetMathLibrary::NormalizedDeltaRotator(FutureRotation, CurrentRotation).Yaw;
}

void UAO_PlayerCharacter_AnimInstance::UpdateEssentialValues()
{
	// Update Character Transform
	CharacterTransformLastFrame = CharacterTransform;
	CharacterTransform = Proxy.Character->GetActorTransform();

	// Update Acceleration
	AccelerationLastFrame = Acceleration;
	Acceleration = Proxy.MovementComponent->GetCurrentAcceleration();
	AccelerationAmount = Acceleration.Length() / Proxy.MovementComponent->GetMaxAcceleration();
	bHasAcceleration = (AccelerationAmount > 0.f);

	// Update Velocity
	VelocityLastFrame = Velocity;
	Velocity = Proxy.MovementComponent->Velocity;
	Speed2D = Velocity.Size2D();
	bHasVelocity = (Speed2D > 5.f);
	VelocityAcceleration = (Velocity - VelocityLastFrame) / GetDeltaSeconds();
}

void UAO_PlayerCharacter_AnimInstance::UpdateStates()
{
	// Update MovementMode (OnGround, InAir)
	MovementModeLastFrame = MovementMode;
	if (Proxy.MovementComponent->MovementMode == MOVE_None
		|| Proxy.MovementComponent->MovementMode == MOVE_Walking
		|| Proxy.MovementComponent->MovementMode == MOVE_Flying)
	{
		MovementMode = ECustomMovementMode::OnGround;
	}
	else if (Proxy.MovementComponent->MovementMode == MOVE_Falling)
	{
		MovementMode = ECustomMovementMode::InAir;
	}

	// Update MovementState (Moving, Idle)
	MovementStateLastFrame = MovementState;
	if (IsMoving())
	{
		MovementState = EMovementState::Moving;
	}
	else
	{
		MovementState = EMovementState::Idle;
	}

	// Update Gait (Run, Walk, Sprint)
	GaitLastFrame = Gait;
	if (Proxy.Character->Gait == EGait::Run)
	{
		Gait = EGait::Run;
	}
	else if (Proxy.Character->Gait == EGait::Walk)
	{
		Gait = EGait::Walk;
	}
	else if (Proxy.Character->Gait == EGait::Sprint)
	{
		Gait = EGait::Sprint;
	}

	// Update Stance (Stand, Crouch)
	StanceLastFrame = Stance;
	if (Proxy.MovementComponent->IsCrouching())
	{
		Stance = EStance::Crouch;
	}
	else
	{
		Stance = EStance::Stand;
	}
}

void UAO_PlayerCharacter_AnimInstance::UpdateTrajectory(float DeltaSeconds)
{
	UAnimInstance* AnimInstance = GetOwningComponent()->GetAnimInstance();

	FPoseSearchTrajectoryData TrajectoryData;
	if (Speed2D > 0.f)
	{
		TrajectoryData = TrajectoryData_Moving;
	}
	else
	{
		TrajectoryData = TrajectoryData_Idle;
	}

	// Generates a prediction trajectory based of the current character intent. For use with Character actors.
	UPoseSearchTrajectoryLibrary::PoseSearchGenerateTransformTrajectory(
		AnimInstance,
		TrajectoryData,
		DeltaSeconds,
		Trajectory,
		PreviousDesiredControllerYaw,
		Trajectory
	);

	// Process InTrajectory to apply gravity and handle collisions. Eventually returns the modified OutTrajectory.
	UPoseSearchTrajectoryLibrary::HandleTransformTrajectoryWorldCollisions(
		this,
		this,
		Trajectory,
		true,
		0.01f,
		Trajectory,
		Trajectory_Collision,
		TraceTypeQuery1,
		false,
		TArray<AActor*>(),
		EDrawDebugTrace::None,
		true,
		150.f
	);

	UPoseSearchTrajectoryLibrary::GetTransformTrajectoryVelocity(Trajectory, -0.3f, -0.2f, TrjPastVelocity, false);
	UPoseSearchTrajectoryLibrary::GetTransformTrajectoryVelocity(Trajectory, 0.f, 0.2f, TrjCurrentVelocity, false);
	UPoseSearchTrajectoryLibrary::GetTransformTrajectoryVelocity(Trajectory, 0.4f, 0.5f, TrjFutureVelocity, false);
}
