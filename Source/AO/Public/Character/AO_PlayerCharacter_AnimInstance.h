// AO_PlayerCharacter_AnimInstance.h

#pragma once

#include "CoreMinimal.h"
#include "AO_PlayerCharacter_MovementEnums.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimInstanceProxy.h"
#include "Animation/TrajectoryTypes.h"
#include "PoseSearch/PoseSearchTrajectoryLibrary.h"
#include "AO_PlayerCharacter_AnimInstance.generated.h"

class UCharacterMovementComponent;
class AAO_PlayerCharacter;
class UPoseSearchDatabase;

USTRUCT()
struct FAOAnimInstanceProxy : public FAnimInstanceProxy
{
	GENERATED_BODY()

protected:
	virtual void InitializeObjects(UAnimInstance* InAnimInstance) override;
	virtual void PreUpdate(UAnimInstance* InAnimInstance, float DeltaSeconds) override;
	virtual void Update(float DeltaSeconds) override;

public:
	UPROPERTY(Transient)
	APawn* Owner = nullptr;
	UPROPERTY(Transient)
	AAO_PlayerCharacter* Character = nullptr;
	UPROPERTY(Transient)
	UCharacterMovementComponent* MovementComponent = nullptr;
};

UCLASS()
class AO_API UAO_PlayerCharacter_AnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
	UPROPERTY(Transient)
	FAOAnimInstanceProxy Proxy;
	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override { return &Proxy; }
	virtual void DestroyAnimInstanceProxy(FAnimInstanceProxy* InProxy) override {}

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;
	
public:
	// Essential Values
	UPROPERTY(BlueprintReadOnly, Category = "PlayerCharacter|Essential Values")
	FTransform CharacterTransform;
	UPROPERTY(BlueprintReadOnly, Category = "PlayerCharacter|Essential Values")
	FTransform CharacterTransformLastFrame;
	
	UPROPERTY(BlueprintReadOnly, Category = "PlayerCharacter|Essential Values")
	FVector Acceleration;
	UPROPERTY(BlueprintReadOnly, Category = "PlayerCharacter|Essential Values")
	FVector AccelerationLastFrame;
	UPROPERTY(BlueprintReadOnly, Category = "PlayerCharacter|Essential Values")
	float AccelerationAmount;
	UPROPERTY(BlueprintReadOnly, Category = "PlayerCharacter|Essential Values")
	bool bHasAcceleration;
	
	UPROPERTY(BlueprintReadOnly, Category = "PlayerCharacter|Essential Values")
	FVector Velocity;
	UPROPERTY(BlueprintReadOnly, Category = "PlayerCharacter|Essential Values")
	FVector VelocityLastFrame;
	UPROPERTY(BlueprintReadOnly, Category = "PlayerCharacter|Essential Values")
	FVector VelocityAcceleration;
	UPROPERTY(BlueprintReadOnly, Category = "PlayerCharacter|Essential Values")
	float Speed2D;
	UPROPERTY(BlueprintReadOnly, Category = "PlayerCharacter|Essential Values")
	bool bHasVelocity;
	UPROPERTY(BlueprintReadOnly, Category = "PlayerCharacter|Essential Values")
	float HeavyLandSpeedThreshold = 700.f;
	
	UPROPERTY(BlueprintReadWrite, Category = "PlayerCharacter|Essential Values")
	UPoseSearchDatabase* CurrentSelectedDatabase;
	UPROPERTY(BlueprintReadWrite, Category = "PlayerCharacter|Essential Values")
	TArray<FName> CurrentDatabaseTags;
	
	// States
	UPROPERTY(BlueprintReadOnly, Category = "PlayerCharacter|States")
	ECustomMovementMode MovementMode = ECustomMovementMode::OnGround;
	UPROPERTY(BlueprintReadOnly, Category = "PlayerCharacter|States")
	ECustomMovementMode MovementModeLastFrame = ECustomMovementMode::OnGround;
	UPROPERTY(BlueprintReadOnly, Category = "PlayerCharacter|States")
	EMovementState MovementState = EMovementState::Idle;
	UPROPERTY(BlueprintReadOnly, Category = "PlayerCharacter|States")
	EMovementState MovementStateLastFrame = EMovementState::Moving;
	UPROPERTY(BlueprintReadOnly, Category = "PlayerCharacter|States")
	EGait Gait = EGait::Run;
	UPROPERTY(BlueprintReadOnly, Category = "PlayerCharacter|States")
	EGait GaitLastFrame = EGait::Run;
	UPROPERTY(BlueprintReadOnly, Category = "PlayerCharacter|States")
	EStance Stance = EStance::Stand;
	UPROPERTY(BlueprintReadOnly, Category = "PlayerCharacter|States")
	EStance StanceLastFrame = EStance::Stand;

	// Trajectory
	UPROPERTY(BlueprintReadOnly, Category = "PlayerCharacter|Trajectory")
	FTransformTrajectory Trajectory;
	UPROPERTY(BlueprintReadOnly, Category = "PlayerCharacter|Trajectory")
	FPoseSearchTrajectoryData TrajectoryData_Idle;
	UPROPERTY(BlueprintReadOnly, Category = "PlayerCharacter|Trajectory")
	FPoseSearchTrajectoryData TrajectoryData_Moving;
	UPROPERTY(BlueprintReadOnly, Category = "PlayerCharacter|Trajectory")
	float PreviousDesiredControllerYaw = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "PlayerCharacter|Trajectory")
	FPoseSearchTrajectory_WorldCollisionResults Trajectory_Collision;
	UPROPERTY(BlueprintReadOnly, Category = "PlayerCharacter|Trajectory")
	FVector TrjPastVelocity;
	UPROPERTY(BlueprintReadOnly, Category = "PlayerCharacter|Trajectory")
	FVector TrjCurrentVelocity;
	UPROPERTY(BlueprintReadOnly, Category = "PlayerCharacter|Trajectory")
	FVector TrjFutureVelocity;
	
public:
	// Movement Analysis
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PlayerCharacter|MovementAnalysis", meta = (BlueprintThreadSafe))
	bool IsMoving();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PlayerCharacter|MovementAnalysis", meta = (BlueprintThreadSafe))
	bool IsStarting();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PlayerCharacter|MovementAnalysis", meta = (BlueprintThreadSafe))
	bool IsPivoting();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PlayerCharacter|MovementAnalysis", meta = (BlueprintThreadSafe))
	bool ShouldTurnInPlace();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PlayerCharacter|MovementAnalysis", meta = (BlueprintThreadSafe))
	bool ShouldSpinTransition();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PlayerCharacter|MovementAnalysis", meta = (BlueprintThreadSafe))
	bool JustTraversal();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PlayerCharacter|MovementAnalysis", meta = (BlueprintThreadSafe))
	bool JustLanded_Heavy();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PlayerCharacter|MovementAnalysis", meta = (BlueprintThreadSafe))
	bool JustLanded_Light();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PlayerCharacter|MovementAnalysis", meta = (BlueprintThreadSafe))
	float GetTrajectoryTurnAngle();
	
protected:
	UFUNCTION(BlueprintCallable, Category = "PlayerCharacter|Update", meta = (BlueprintThreadSafe))
	void UpdateEssentialValues();
	UFUNCTION(BlueprintCallable, Category = "PlayerCharacter|Update", meta = (BlueprintThreadSafe))
	void UpdateStates();
	UFUNCTION(BlueprintCallable, Category = "PlayerCharacter|Update", meta = (BlueprintThreadSafe))
	void UpdateTrajectory(float DeltaSeconds);
};
