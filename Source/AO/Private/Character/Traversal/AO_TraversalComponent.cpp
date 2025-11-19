// AO_TraversalComponent.cpp - KH

#include "Character/Traversal/AO_TraversalComponent.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "ChooserFunctionLibrary.h"
#include "PoseSearch/PoseSearchLibrary.h"
#include "MotionWarpingComponent.h"
#include "AnimationWarpingLibrary.h"

#include "AO_Log.h"
#include "Character/AO_PlayerCharacter.h"
#include "Maps/Traversal/AO_TraversableComponent.h"

UAO_TraversalComponent::UAO_TraversalComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UAO_TraversalComponent::BeginPlay()
{
	Super::BeginPlay();

	Owner = GetOwner();
	if (!Owner)
	{
		AO_LOG(LogKH, Error, TEXT("Failed to get Owner"));
		return;
	}

	Character = Cast<ACharacter>(Owner);
	if (!Character)
	{
		AO_LOG(LogKH, Error, TEXT("Failed to cast Owner to ACharacter"));
		return;
	}

	CharacterMovement = Character->GetCharacterMovement();
	if (!CharacterMovement)
	{
		AO_LOG(LogKH, Error, TEXT("Failed to get CharacterMovementComponent"));
		return;
	}
}

bool UAO_TraversalComponent::TryTraversal()
{
	if (bDoingTraversal)
	{
		AO_LOG(LogKH, Warning, TEXT("Traversal is already in progress"));
		return false;
	}
	
	if (!GetTraversalCheckInputs())
	{
		AO_LOG(LogKH, Warning, TEXT("Failed to get inputs for traversal check"));
		return false;
	}

	FVector ActorLocation = GetOwner()->GetActorLocation();
	UCapsuleComponent* CapsuleComponent = Cast<UCapsuleComponent>(GetOwner()->GetComponentByClass(UCapsuleComponent::StaticClass()));
	if (!CapsuleComponent)
	{
		AO_LOG(LogKH, Warning, TEXT("Failed to get CapsuleComponent"));
		return false;
	}
	float CapsuleRadius = CapsuleComponent->GetScaledCapsuleRadius();
	float CapsuleHalfHeight = CapsuleComponent->GetScaledCapsuleHalfHeight();

	// Perform a trace in the actor's forward direction to find a traversable object
	FHitResult HitResult;
	FVector TraceStart = ActorLocation + TraversalInput.TraceOriginOffset;
	FVector TraceEnd = TraceStart + TraversalInput.TraceForwardDirection * TraversalInput.TraceForwardDistance + TraversalInput.TraceEndOffset;

	RunCapsuleTrace(
		TraceStart,
		TraceEnd,
		TraversalInput.TraceRadius,
		TraversalInput.TraceHalfHeight,
		HitResult,
		FColor::Black,
		FColor::Green);

	if (!HitResult.bBlockingHit)
	{
		AO_LOG(LogKH, Display, TEXT("No blocking hit found"));
		return false;
	}

	// Get the front and back ledge transforms from the traversable component
	UAO_TraversableComponent* TraversableComponent = Cast<UAO_TraversableComponent>(
		HitResult.GetActor()->GetComponentByClass(UAO_TraversableComponent::StaticClass()));
	if (!TraversableComponent)
	{
		AO_LOG(LogKH, Display, TEXT("No TraversableComponent found on hit actor"));
		return false;
	}

	TraversalResult.HitComponent = HitResult.GetComponent();
	TraversableComponent->GetLedgeTransforms(HitResult.ImpactPoint, ActorLocation, TraversalResult);

	if (!TraversalResult.bHasFrontLedge)
	{
		AO_LOG(LogKH, Display, TEXT("No front ledge found"));
		return false;
	}
	
	if (DrawDebugLevel >= 1)
	{
		if (TraversalResult.bHasFrontLedge)
		{
			DrawDebugSphere(GetWorld(), TraversalResult.FrontLedgeLocation, 10.f, 12,	FColor::Green, false, DrawDebugDuration, 0, 1.0f);
		}

		if (TraversalResult.bHasBackLedge)
		{
			DrawDebugSphere(GetWorld(), TraversalResult.BackLedgeLocation, 10.f, 12,	FColor::Cyan, false, DrawDebugDuration, 0, 1.0f);
		}
		else
		{
			AO_LOG(LogKH, Display, TEXT("No back ledge found"));
		}
	}
	
	// Perform a trace from the actors location up to the front ledge location to determine if there is room for the actor to move up to it
	FVector RoomCheckFrontLedgeLocation = TraversalResult.FrontLedgeLocation + TraversalResult.FrontLedgeNormal * (CapsuleRadius + 2.0f);
	RoomCheckFrontLedgeLocation.Z += CapsuleHalfHeight + 2.0f;

	FHitResult RoomCheckFrontHitResult;
	RunCapsuleTrace(
		ActorLocation,
		RoomCheckFrontLedgeLocation,
		CapsuleRadius,
		CapsuleHalfHeight,
		RoomCheckFrontHitResult,
		FColor::Red,
		FColor::Green);

	if (RoomCheckFrontHitResult.bBlockingHit)
	{
		AO_LOG(LogKH, Display, TEXT("No room for actor to move up to front ledge"));
		return false;
	}

	// Save the height of the obstacle
	TraversalResult.ObstacleHeight = abs((ActorLocation.Z - CapsuleHalfHeight) - TraversalResult.FrontLedgeLocation.Z);
	
	// Perform a trace across the top of the obstacle from the front ledge to the back ledge to see if theres room for the actor to move across it.
	FVector RoomCheckBackLedgeLocation = TraversalResult.BackLedgeLocation + TraversalResult.BackLedgeNormal * (CapsuleRadius + 2.0f);
	RoomCheckBackLedgeLocation.Z += CapsuleHalfHeight + 2.0f;

	FHitResult RoomCheckBackHitResult;
	bool bRoomCheckBackHit = RunCapsuleTrace(
		RoomCheckFrontLedgeLocation,
		RoomCheckBackLedgeLocation,
		CapsuleRadius,
		CapsuleHalfHeight,
		RoomCheckBackHitResult,
		FColor::Red,
		FColor::Green);

	if (bRoomCheckBackHit)
	{
		TraversalResult.ObstacleDepth = (RoomCheckBackHitResult.ImpactPoint - TraversalResult.FrontLedgeLocation).Size2D();
		TraversalResult.bHasBackLedge = false;
	}
	else
	{
		TraversalResult.ObstacleDepth = (TraversalResult.FrontLedgeLocation - TraversalResult.BackLedgeLocation).Size2D();

		// Trace downward from the back ledge location to find the floor
		FVector BackFloorTrace = TraversalResult.BackLedgeLocation + TraversalResult.BackLedgeNormal * (CapsuleRadius + 2.0f);
		BackFloorTrace.Z -= 50.f;
		FHitResult BackFloorHitResult;
		RunCapsuleTrace(
			RoomCheckBackLedgeLocation,
			BackFloorTrace,
			CapsuleRadius,
			CapsuleHalfHeight,
			BackFloorHitResult,
			FColor::Red,
			FColor::Green);

		if (BackFloorHitResult.bBlockingHit)
		{
			TraversalResult.bHasBackFloor = true;
			TraversalResult.BackFloorLocation = BackFloorHitResult.ImpactPoint;
			TraversalResult.BackLedgeHeight = abs(BackFloorHitResult.ImpactPoint.Z - TraversalResult.BackLedgeLocation.Z);
		}
		else
		{
			TraversalResult.bHasBackFloor = false;
		}
	}
	
	// Evaluate a chooser to select all montages that match the conditions of the traversal check.
	AAO_PlayerCharacter* PlayerCharacter = Cast<AAO_PlayerCharacter>(Owner);
	if (!PlayerCharacter)
	{
		AO_LOG(LogKH, Warning, TEXT("Failed to cast Owner to AAO_PlayerCharacter"));
		return false;
	}

	if (!AnimChooserTable)
	{
		AO_LOG(LogKH, Warning, TEXT("AnimChooserTable is null"));
		return false;
	}

	const FInstancedStruct ResultInstances = UChooserFunctionLibrary::MakeEvaluateChooser(AnimChooserTable);
	FChooserEvaluationContext EvaluationContext;

	FInstancedStruct InputStruct;
	InputStruct.InitializeAs<FTraversalChooserInput>();
	FTraversalChooserInput& InputData = InputStruct.GetMutable<FTraversalChooserInput>();
	FTraversalChooserInput ChooserInput = FTraversalChooserInput(
		TraversalResult.ActionType,
		TraversalResult.bHasFrontLedge,
		TraversalResult.bHasBackLedge,
		TraversalResult.bHasBackFloor,
		TraversalResult.ObstacleHeight,
		TraversalResult.ObstacleDepth,
		TraversalResult.BackLedgeHeight,
		CharacterMovement->MovementMode,
		PlayerCharacter->Gait,
		CharacterMovement->Velocity.Size2D());
	InputData = ChooserInput;
	EvaluationContext.Params.Add(InputStruct);

	FInstancedStruct OutputStruct;
	OutputStruct.InitializeAs<FTraversalChooserOutput>();
	FTraversalChooserOutput& OutputData = OutputStruct.GetMutable<FTraversalChooserOutput>();
	EvaluationContext.Params.Add(OutputStruct);
	
	TArray<UObject*> EvaluateObjects = UChooserFunctionLibrary::EvaluateObjectChooserBaseMulti(
		 EvaluationContext, ResultInstances, UAnimMontage::StaticClass());

	TraversalResult.ActionType = OutputData.ActionType;
	if (TraversalResult.ActionType == ETraversalActionType::None)
	{
		AO_LOG(LogKH, Display, TEXT("No valid traversal action found"));
		return false;
	}

	// Debug Traversal Result
	AO_LOG(LogKH, Display, TEXT("Has Front Ledge: %s"), TraversalResult.bHasFrontLedge ? TEXT("true") : TEXT("false"));
	AO_LOG(LogKH, Display, TEXT("Has Back Ledge: %s"), TraversalResult.bHasBackLedge ? TEXT("true") : TEXT("false"));
	AO_LOG(LogKH, Display, TEXT("Has Back Floor: %s"), TraversalResult.bHasBackFloor ? TEXT("true") : TEXT("false"));
	AO_LOG(LogKH, Display, TEXT("Obstacle Height: %f"), TraversalResult.ObstacleHeight);
	AO_LOG(LogKH, Display, TEXT("Obstacle Depth: %f"), TraversalResult.ObstacleDepth);
	AO_LOG(LogKH, Display, TEXT("Back Ledge Height: %f"), TraversalResult.BackLedgeHeight);

	// Perform a Motion Match on all the montages that were chosen by the chooser to find the best result.
	// This match will elect the best montage AND the best entry frame (start time) based on the distance to the ledge, and the current characters pose.
	FPoseSearchContinuingProperties ContinuingProperties;
	FPoseSearchFutureProperties FutureProperties;
	FPoseSearchBlueprintResult MotionMatchResult;
	UPoseSearchLibrary::MotionMatch(
		Character->GetMesh()->GetAnimInstance(),
		EvaluateObjects,
		FName(TEXT("PoseHistory")),
		ContinuingProperties,
		FutureProperties,
		MotionMatchResult);

	UAnimMontage* SelectedAnim = Cast<UAnimMontage>(MotionMatchResult.SelectedAnim);
	if (!SelectedAnim)
	{
		AO_LOG(LogKH, Warning, TEXT("Failed to cast SelectedAnim to UAnimMontage"));
		return false;
	}
	AO_LOG(LogKH, Display, TEXT("Selected Anim: %s"), *SelectedAnim->GetName());
	
	TraversalResult.ChosenMontage = SelectedAnim;
	TraversalResult.StartTime = MotionMatchResult.SelectedTime;
	TraversalResult.PlayRate = MotionMatchResult.WantedPlayRate;

	// Perform the traversal animation
	PerformTraversalAnimation();
	
	return true;
}

bool UAO_TraversalComponent::GetTraversalCheckInputs()
{
	if (!Owner || !Character || !CharacterMovement)
	{
		return false;
	}
	
	if (CharacterMovement->MovementMode == MOVE_None
		|| CharacterMovement->MovementMode == MOVE_Walking
		|| CharacterMovement->MovementMode == MOVE_Custom)
	{
		FVector TraceDirection = UKismetMathLibrary::LessLess_VectorRotator(CharacterMovement->Velocity, Owner->GetActorRotation());
		TraversalInput.TraceForwardDistance = UKismetMathLibrary::MapRangeClamped(TraceDirection.X, 0.f, 500.f, 75.f, 350.f);
		TraversalInput.TraceForwardDirection = Owner->GetActorForwardVector();
		TraversalInput.TraceEndOffset.Z = 0.f;
		TraversalInput.TraceRadius = 30.f;
		TraversalInput.TraceHalfHeight = 60.f;
		return true;
	}
	
	if (CharacterMovement->MovementMode == MOVE_Falling
		|| CharacterMovement->MovementMode == MOVE_Flying)
	{
		TraversalInput.TraceForwardDirection = Owner->GetActorForwardVector();
		TraversalInput.TraceForwardDistance = 75.f;
		TraversalInput.TraceEndOffset.Z = 50.f;
		TraversalInput.TraceRadius = 30.f;
		TraversalInput.TraceHalfHeight = 86.f;
		return true;
	}

	return false;
}

bool UAO_TraversalComponent::RunCapsuleTrace(const FVector& StartLocation, const FVector& EndLocation, float Radius,
	float HalfHeight, FHitResult& OutHit, FColor DebugHitColor, FColor DebugTraceColor)
{
	TObjectPtr<UWorld> World = GetWorld();
	if (!World)
	{
		return false;
	}
	
	FCollisionShape TraceShape = FCollisionShape::MakeCapsule(Radius, HalfHeight);
	FCollisionQueryParams CollisionResponseParams(FName(TEXT("TraversalTrace")), false, GetOwner());
	bool bHit = World->SweepSingleByChannel(
		OutHit,
		StartLocation,
		EndLocation,
		FQuat::Identity,
		ECC_Visibility,
		TraceShape,
		CollisionResponseParams
	);

	if (DrawDebugLevel >= 2)
	{
		FQuat CapsuleRot = FQuat::Identity;

		if (bHit)
		{
			// 충돌 지점까지의 스윕 캡슐
			DrawDebugCapsule(World, OutHit.Location, HalfHeight, Radius, CapsuleRot, DebugHitColor, false, DrawDebugDuration);
			
			// 충돌이 시작된 곳에서 충돌 지점까지의 선
			DrawDebugLine(World, StartLocation, OutHit.Location, DebugHitColor, false, DrawDebugDuration);
		}
		else
		{
			// 전체 스윕 경로의 끝 지점 캡슐
			DrawDebugCapsule(World, EndLocation, HalfHeight, Radius, CapsuleRot, DebugTraceColor, false, DrawDebugDuration);

			// 전체 트레이스 선 (Start -> End)
			DrawDebugLine(World, StartLocation, EndLocation, DebugTraceColor, false, DrawDebugDuration);
		}
	}

	return bHit;
}

void UAO_TraversalComponent::PerformTraversalAnimation()
{
	if (!Character || !CharacterMovement)
	{
		AO_LOG(LogKH, Error, TEXT("Character or CharacterMovementComponent is null"));
		return;
	}

	// 모션 워핑 실행
	UpdateWarpTargets();

	// 서버 권위를 일시적으로 완화하고, 클라이언트 측 예측을 극대화하기 위해 사용
	CharacterMovement->bIgnoreClientMovementErrorChecksAndCorrection = true;
	CharacterMovement->bServerAcceptClientAuthoritativePosition = true;

	TObjectPtr<UAnimInstance> AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (!AnimInstance || !TraversalResult.ChosenMontage)
	{
		AO_LOG(LogKH, Error, TEXT("AnimInstance or TraversalResult.ChosenMontage is null"));
		return;
	}
	AnimInstance->Montage_Play(
		TraversalResult.ChosenMontage,
		TraversalResult.PlayRate,
		EMontagePlayReturnType::MontageLength,
		TraversalResult.StartTime);

	// 몽타주 종료 시에 실행할 함수 바인딩
	FOnMontageBlendingOutStarted BlendOutingDelegate;
	BlendOutingDelegate.BindUFunction(this, FName("OnMontageBlendingOut"));
	AnimInstance->Montage_SetBlendingOutDelegate(BlendOutingDelegate, TraversalResult.ChosenMontage);

	bDoingTraversal = true;

	TObjectPtr<UCapsuleComponent> CapsuleComponent = Cast<UCapsuleComponent>(
		GetOwner()->GetComponentByClass(UCapsuleComponent::StaticClass()));
	if (!CapsuleComponent)
	{
		AO_LOG(LogKH, Error, TEXT("Failed to get CapsuleComponent"));
		return;
	}
	CapsuleComponent->IgnoreComponentWhenMoving(TraversalResult.HitComponent, true);

	CharacterMovement->SetMovementMode(MOVE_Flying);
}

void UAO_TraversalComponent::UpdateWarpTargets()
{
	TObjectPtr<UMotionWarpingComponent> MotionWarping = Cast<UMotionWarpingComponent>(
		Owner->GetComponentByClass(UMotionWarpingComponent::StaticClass()));
	if (!MotionWarping)
	{
		AO_LOG(LogKH, Warning, TEXT("Failed to get MotionWarpingComponent"));
		return;
	}

	MotionWarping->AddOrUpdateWarpTargetFromLocationAndRotation(
		FName(TEXT("FrontLedge")),
		TraversalResult.FrontLedgeLocation + FVector(0.f, 0.f, 0.5f),
		UKismetMathLibrary::MakeRotFromX(UKismetMathLibrary::NegateVector(TraversalResult.FrontLedgeNormal)));

	float DistanceFromFrontLedgeToBackLedge = 0.f;
	float DistanceFromFrontLedgeToBackFloor = 0.f;
	
	if (TraversalResult.ActionType == ETraversalActionType::Hurdle
		|| TraversalResult.ActionType == ETraversalActionType::Vault)
	{
		TArray<FMotionWarpingWindowData> OutWindows;
		UMotionWarpingUtilities::GetMotionWarpingWindowsForWarpTargetFromAnimation(TraversalResult.ChosenMontage, FName(TEXT("BackLedge")), OutWindows);

		if (OutWindows.Num() > 0)
		{
			UAnimationWarpingLibrary::GetCurveValueFromAnimation(
				TraversalResult.ChosenMontage,
				FName(TEXT("Distance_From_Ledge")),
				OutWindows[0].EndTime,
				DistanceFromFrontLedgeToBackLedge);

			MotionWarping->AddOrUpdateWarpTargetFromLocationAndRotation(
				FName(TEXT("BackLedge")),
				TraversalResult.BackLedgeLocation,
				FRotator::ZeroRotator);
		}
		else
		{
			MotionWarping->RemoveWarpTarget(FName(TEXT("BackLedge")));
		}
	}
	else
	{
		MotionWarping->RemoveWarpTarget(FName(TEXT("BackLedge")));
	}

	if (TraversalResult.ActionType == ETraversalActionType::Hurdle)
	{
		TArray<FMotionWarpingWindowData> OutWindows;
		UMotionWarpingUtilities::GetMotionWarpingWindowsForWarpTargetFromAnimation(TraversalResult.ChosenMontage, FName(TEXT("BackFloor")), OutWindows);

		if (OutWindows.Num() > 0)
		{
			UAnimationWarpingLibrary::GetCurveValueFromAnimation(
				TraversalResult.ChosenMontage,
				FName(TEXT("Distance_From_Ledge")),
				OutWindows[0].EndTime,
				DistanceFromFrontLedgeToBackFloor);

			FVector TargetLocation = TraversalResult.BackLedgeLocation
				+ TraversalResult.BackLedgeNormal * abs(DistanceFromFrontLedgeToBackLedge - DistanceFromFrontLedgeToBackFloor);
			TargetLocation.Z = TraversalResult.BackFloorLocation.Z;
			
			MotionWarping->AddOrUpdateWarpTargetFromLocationAndRotation(
				FName(TEXT("BackFloor")),
				TargetLocation,
				FRotator::ZeroRotator);
		}
		else
		{
			MotionWarping->RemoveWarpTarget(FName(TEXT("BackFloor")));
		}
	}
	else
	{
		MotionWarping->RemoveWarpTarget(FName(TEXT("BackFloor")));
	}
}

void UAO_TraversalComponent::OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
	bDoingTraversal = false;

	TObjectPtr<UCapsuleComponent> CapsuleComponent = Cast<UCapsuleComponent>(
		GetOwner()->GetComponentByClass(UCapsuleComponent::StaticClass()));
	if (!CapsuleComponent)
	{
		AO_LOG(LogKH, Error, TEXT("Failed to get CapsuleComponent"));
		return;
	}
	CapsuleComponent->IgnoreComponentWhenMoving(TraversalResult.HitComponent, false);
	
	if (TraversalResult.ActionType == ETraversalActionType::Vault)
	{
		CharacterMovement->SetMovementMode(MOVE_Falling);
	}
	else
	{
		CharacterMovement->SetMovementMode(MOVE_Walking);
	}

	// 서버 권위를 다시 활성화
	CharacterMovement->bIgnoreClientMovementErrorChecksAndCorrection = false;
	CharacterMovement->bServerAcceptClientAuthoritativePosition = false;
}
